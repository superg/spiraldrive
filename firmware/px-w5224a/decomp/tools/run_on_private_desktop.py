#!/usr/bin/env python3
"""Run a Windows process tree on a non-interactive desktop.

This is intended for GUI-backed command-line automation such as Keil uVision.
The program can still create and pump windows, but those windows cannot become
foreground windows on the user's normal desktop.
"""

from __future__ import annotations

import argparse
import ctypes
from ctypes import wintypes
import os
from pathlib import Path
import shutil
import subprocess
import sys
import uuid


BELOW_NORMAL_PRIORITY_CLASS = 0x00004000
CREATE_NO_WINDOW = 0x08000000
CREATE_SUSPENDED = 0x00000004
GENERIC_ALL = 0x10000000
INFINITE = 0xFFFFFFFF
JOB_OBJECT_EXTENDED_LIMIT_INFORMATION_CLASS = 9
JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE = 0x00002000
STARTF_USESHOWWINDOW = 0x00000001
STARTF_USESTDHANDLES = 0x00000100
STD_ERROR_HANDLE = -12
STD_INPUT_HANDLE = -10
STD_OUTPUT_HANDLE = -11
SW_HIDE = 0
WAIT_FAILED = 0xFFFFFFFF
WAIT_OBJECT_0 = 0
JOB_OBJECT_BASIC_PROCESS_ID_LIST_CLASS = 3
PROCESS_QUERY_LIMITED_INFORMATION = 0x1000
PROCESS_TERMINATE = 0x0001
WATCHDOG_PROCESS_CAPACITY = 64


class STARTUPINFOW(ctypes.Structure):
    _fields_ = [
        ("cb", wintypes.DWORD),
        ("lpReserved", wintypes.LPWSTR),
        ("lpDesktop", wintypes.LPWSTR),
        ("lpTitle", wintypes.LPWSTR),
        ("dwX", wintypes.DWORD),
        ("dwY", wintypes.DWORD),
        ("dwXSize", wintypes.DWORD),
        ("dwYSize", wintypes.DWORD),
        ("dwXCountChars", wintypes.DWORD),
        ("dwYCountChars", wintypes.DWORD),
        ("dwFillAttribute", wintypes.DWORD),
        ("dwFlags", wintypes.DWORD),
        ("wShowWindow", wintypes.WORD),
        ("cbReserved2", wintypes.WORD),
        ("lpReserved2", ctypes.POINTER(wintypes.BYTE)),
        ("hStdInput", wintypes.HANDLE),
        ("hStdOutput", wintypes.HANDLE),
        ("hStdError", wintypes.HANDLE),
    ]


class PROCESS_INFORMATION(ctypes.Structure):
    _fields_ = [
        ("hProcess", wintypes.HANDLE),
        ("hThread", wintypes.HANDLE),
        ("dwProcessId", wintypes.DWORD),
        ("dwThreadId", wintypes.DWORD),
    ]


class IO_COUNTERS(ctypes.Structure):
    _fields_ = [
        ("ReadOperationCount", ctypes.c_ulonglong),
        ("WriteOperationCount", ctypes.c_ulonglong),
        ("OtherOperationCount", ctypes.c_ulonglong),
        ("ReadTransferCount", ctypes.c_ulonglong),
        ("WriteTransferCount", ctypes.c_ulonglong),
        ("OtherTransferCount", ctypes.c_ulonglong),
    ]


class JOBOBJECT_BASIC_LIMIT_INFORMATION(ctypes.Structure):
    _fields_ = [
        ("PerProcessUserTimeLimit", ctypes.c_longlong),
        ("PerJobUserTimeLimit", ctypes.c_longlong),
        ("LimitFlags", wintypes.DWORD),
        ("MinimumWorkingSetSize", ctypes.c_size_t),
        ("MaximumWorkingSetSize", ctypes.c_size_t),
        ("ActiveProcessLimit", wintypes.DWORD),
        ("Affinity", ctypes.c_size_t),
        ("PriorityClass", wintypes.DWORD),
        ("SchedulingClass", wintypes.DWORD),
    ]


class JOBOBJECT_EXTENDED_LIMIT_INFORMATION(ctypes.Structure):
    _fields_ = [
        ("BasicLimitInformation", JOBOBJECT_BASIC_LIMIT_INFORMATION),
        ("IoInfo", IO_COUNTERS),
        ("ProcessMemoryLimit", ctypes.c_size_t),
        ("JobMemoryLimit", ctypes.c_size_t),
        ("PeakProcessMemoryUsed", ctypes.c_size_t),
        ("PeakJobMemoryUsed", ctypes.c_size_t),
    ]


class JOBOBJECT_BASIC_PROCESS_ID_LIST(ctypes.Structure):
    _fields_ = [
        ("NumberOfAssignedProcesses", wintypes.DWORD),
        ("NumberOfProcessIdsInList", wintypes.DWORD),
        ("ProcessIdList", ctypes.c_size_t * WATCHDOG_PROCESS_CAPACITY),
    ]


def windows_error(message: str) -> OSError:
    error = ctypes.get_last_error()
    return OSError(error, f"{message}: {ctypes.FormatError(error).strip()}")


def resolved_executable(command: list[str]) -> str:
    candidate = Path(command[0])
    if candidate.is_file():
        return str(candidate.resolve())
    found = shutil.which(command[0])
    if found is None:
        raise FileNotFoundError(command[0])
    return str(Path(found).resolve())


def process_age_seconds(kernel32: object, process: wintypes.HANDLE) -> float | None:
    creation = wintypes.FILETIME()
    exit_time = wintypes.FILETIME()
    kernel = wintypes.FILETIME()
    user = wintypes.FILETIME()
    if not kernel32.GetProcessTimes(
        process,
        ctypes.byref(creation),
        ctypes.byref(exit_time),
        ctypes.byref(kernel),
        ctypes.byref(user),
    ):
        return None
    now = wintypes.FILETIME()
    kernel32.GetSystemTimeAsFileTime(ctypes.byref(now))
    created_value = (creation.dwHighDateTime << 32) | creation.dwLowDateTime
    now_value = (now.dwHighDateTime << 32) | now.dwLowDateTime
    return max(0.0, (now_value - created_value) / 10_000_000.0)


def terminate_overdue_job_processes(
    kernel32: object,
    job: wintypes.HANDLE,
    root_process_id: int,
    executable_name: str,
    maximum_age: float,
    already_terminated: set[int],
) -> None:
    process_ids = JOBOBJECT_BASIC_PROCESS_ID_LIST()
    returned = wintypes.DWORD()
    if not kernel32.QueryInformationJobObject(
        job,
        JOB_OBJECT_BASIC_PROCESS_ID_LIST_CLASS,
        ctypes.byref(process_ids),
        ctypes.sizeof(process_ids),
        ctypes.byref(returned),
    ):
        raise windows_error("QueryInformationJobObject failed")
    if process_ids.NumberOfAssignedProcesses > WATCHDOG_PROCESS_CAPACITY:
        raise RuntimeError("private-desktop job exceeded watchdog process capacity")

    expected_name = executable_name.casefold()
    for index in range(process_ids.NumberOfProcessIdsInList):
        process_id = int(process_ids.ProcessIdList[index])
        if process_id == root_process_id or process_id in already_terminated:
            continue
        child = kernel32.OpenProcess(
            PROCESS_QUERY_LIMITED_INFORMATION | PROCESS_TERMINATE,
            False,
            process_id,
        )
        if not child:
            continue
        try:
            path_buffer = ctypes.create_unicode_buffer(32768)
            path_length = wintypes.DWORD(len(path_buffer))
            if not kernel32.QueryFullProcessImageNameW(
                child, 0, path_buffer, ctypes.byref(path_length)
            ):
                continue
            if Path(path_buffer.value).name.casefold() != expected_name:
                continue
            age = process_age_seconds(kernel32, child)
            if age is None or age <= maximum_age:
                continue
            if not kernel32.TerminateProcess(child, 0xDEAD):
                raise windows_error(f"TerminateProcess failed for PID {process_id}")
            already_terminated.add(process_id)
            print(
                f"private-desktop watchdog terminated {executable_name} "
                f"PID {process_id} after {age:.1f}s",
                file=sys.stderr,
                flush=True,
            )
        finally:
            kernel32.CloseHandle(child)


def run_on_private_desktop(
    command: list[str],
    cwd: Path,
    watch_executable: str | None = None,
    watchdog_seconds: float | None = None,
) -> int:
    if os.name != "nt":
        raise RuntimeError("private Windows desktops are only available on Windows")
    if not command:
        raise ValueError("missing command")

    user32 = ctypes.WinDLL("user32", use_last_error=True)
    kernel32 = ctypes.WinDLL("kernel32", use_last_error=True)

    user32.CreateDesktopW.argtypes = [
        wintypes.LPCWSTR,
        wintypes.LPCWSTR,
        ctypes.c_void_p,
        wintypes.DWORD,
        wintypes.DWORD,
        ctypes.c_void_p,
    ]
    user32.CreateDesktopW.restype = wintypes.HANDLE
    user32.CloseDesktop.argtypes = [wintypes.HANDLE]
    user32.CloseDesktop.restype = wintypes.BOOL
    kernel32.CreateJobObjectW.argtypes = [ctypes.c_void_p, wintypes.LPCWSTR]
    kernel32.CreateJobObjectW.restype = wintypes.HANDLE
    kernel32.SetInformationJobObject.argtypes = [
        wintypes.HANDLE,
        ctypes.c_int,
        ctypes.c_void_p,
        wintypes.DWORD,
    ]
    kernel32.SetInformationJobObject.restype = wintypes.BOOL
    kernel32.CreateProcessW.argtypes = [
        wintypes.LPCWSTR,
        wintypes.LPWSTR,
        ctypes.c_void_p,
        ctypes.c_void_p,
        wintypes.BOOL,
        wintypes.DWORD,
        ctypes.c_void_p,
        wintypes.LPCWSTR,
        ctypes.POINTER(STARTUPINFOW),
        ctypes.POINTER(PROCESS_INFORMATION),
    ]
    kernel32.CreateProcessW.restype = wintypes.BOOL
    kernel32.AssignProcessToJobObject.argtypes = [wintypes.HANDLE, wintypes.HANDLE]
    kernel32.AssignProcessToJobObject.restype = wintypes.BOOL
    kernel32.QueryInformationJobObject.argtypes = [
        wintypes.HANDLE,
        ctypes.c_int,
        ctypes.c_void_p,
        wintypes.DWORD,
        ctypes.POINTER(wintypes.DWORD),
    ]
    kernel32.QueryInformationJobObject.restype = wintypes.BOOL
    kernel32.OpenProcess.argtypes = [wintypes.DWORD, wintypes.BOOL, wintypes.DWORD]
    kernel32.OpenProcess.restype = wintypes.HANDLE
    kernel32.QueryFullProcessImageNameW.argtypes = [
        wintypes.HANDLE,
        wintypes.DWORD,
        wintypes.LPWSTR,
        ctypes.POINTER(wintypes.DWORD),
    ]
    kernel32.QueryFullProcessImageNameW.restype = wintypes.BOOL
    kernel32.GetProcessTimes.argtypes = [
        wintypes.HANDLE,
        ctypes.POINTER(wintypes.FILETIME),
        ctypes.POINTER(wintypes.FILETIME),
        ctypes.POINTER(wintypes.FILETIME),
        ctypes.POINTER(wintypes.FILETIME),
    ]
    kernel32.GetProcessTimes.restype = wintypes.BOOL
    kernel32.GetSystemTimeAsFileTime.argtypes = [ctypes.POINTER(wintypes.FILETIME)]
    kernel32.GetSystemTimeAsFileTime.restype = None
    kernel32.ResumeThread.argtypes = [wintypes.HANDLE]
    kernel32.ResumeThread.restype = wintypes.DWORD
    kernel32.WaitForSingleObject.argtypes = [wintypes.HANDLE, wintypes.DWORD]
    kernel32.WaitForSingleObject.restype = wintypes.DWORD
    kernel32.GetExitCodeProcess.argtypes = [wintypes.HANDLE, ctypes.POINTER(wintypes.DWORD)]
    kernel32.GetExitCodeProcess.restype = wintypes.BOOL
    kernel32.GetStdHandle.argtypes = [wintypes.DWORD]
    kernel32.GetStdHandle.restype = wintypes.HANDLE
    kernel32.TerminateProcess.argtypes = [wintypes.HANDLE, wintypes.UINT]
    kernel32.TerminateProcess.restype = wintypes.BOOL
    kernel32.CloseHandle.argtypes = [wintypes.HANDLE]
    kernel32.CloseHandle.restype = wintypes.BOOL

    desktop_name = f"plextor_uv4_{os.getpid()}_{uuid.uuid4().hex}"
    desktop = user32.CreateDesktopW(desktop_name, None, None, 0, GENERIC_ALL, None)
    if not desktop:
        raise windows_error("CreateDesktopW failed")

    job = kernel32.CreateJobObjectW(None, None)
    if not job:
        user32.CloseDesktop(desktop)
        raise windows_error("CreateJobObjectW failed")

    limits = JOBOBJECT_EXTENDED_LIMIT_INFORMATION()
    limits.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE
    if not kernel32.SetInformationJobObject(
        job,
        JOB_OBJECT_EXTENDED_LIMIT_INFORMATION_CLASS,
        ctypes.byref(limits),
        ctypes.sizeof(limits),
    ):
        kernel32.CloseHandle(job)
        user32.CloseDesktop(desktop)
        raise windows_error("SetInformationJobObject failed")

    startup = STARTUPINFOW()
    startup.cb = ctypes.sizeof(startup)
    startup.lpDesktop = f"WinSta0\\{desktop_name}"
    startup.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES
    startup.wShowWindow = SW_HIDE
    startup.hStdInput = kernel32.GetStdHandle(STD_INPUT_HANDLE)
    startup.hStdOutput = kernel32.GetStdHandle(STD_OUTPUT_HANDLE)
    startup.hStdError = kernel32.GetStdHandle(STD_ERROR_HANDLE)
    process = PROCESS_INFORMATION()
    executable = resolved_executable(command)
    command_line = ctypes.create_unicode_buffer(subprocess.list2cmdline(command))
    created = kernel32.CreateProcessW(
        executable,
        command_line,
        None,
        None,
        True,
        CREATE_SUSPENDED | CREATE_NO_WINDOW | BELOW_NORMAL_PRIORITY_CLASS,
        None,
        str(cwd.resolve()),
        ctypes.byref(startup),
        ctypes.byref(process),
    )
    if not created:
        kernel32.CloseHandle(job)
        user32.CloseDesktop(desktop)
        raise windows_error("CreateProcessW failed")

    exit_code = wintypes.DWORD(1)
    watchdog_terminations: set[int] = set()
    try:
        if not kernel32.AssignProcessToJobObject(job, process.hProcess):
            kernel32.TerminateProcess(process.hProcess, 1)
            raise windows_error("AssignProcessToJobObject failed")
        if kernel32.ResumeThread(process.hThread) == WAIT_FAILED:
            kernel32.TerminateProcess(process.hProcess, 1)
            raise windows_error("ResumeThread failed")
        while True:
            wait = kernel32.WaitForSingleObject(process.hProcess, 250)
            if wait == WAIT_OBJECT_0:
                break
            if wait == WAIT_FAILED:
                raise windows_error("WaitForSingleObject failed")
            if watch_executable is not None and watchdog_seconds is not None:
                terminate_overdue_job_processes(
                    kernel32,
                    job,
                    int(process.dwProcessId),
                    watch_executable,
                    watchdog_seconds,
                    watchdog_terminations,
                )
        if not kernel32.GetExitCodeProcess(process.hProcess, ctypes.byref(exit_code)):
            raise windows_error("GetExitCodeProcess failed")
        return int(exit_code.value)
    except KeyboardInterrupt:
        # Closing the kill-on-close job below terminates the runner and any
        # currently active uVision descendant as one process tree.
        return 130
    finally:
        kernel32.CloseHandle(process.hThread)
        kernel32.CloseHandle(process.hProcess)
        kernel32.CloseHandle(job)
        user32.CloseDesktop(desktop)


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--watch-executable")
    parser.add_argument("--watchdog-seconds", type=float)
    parser.add_argument("command", nargs=argparse.REMAINDER)
    args = parser.parse_args(argv)
    command = args.command
    if command and command[0] == "--":
        command = command[1:]
    if not command:
        parser.error("a command is required after --")
    if (args.watch_executable is None) != (args.watchdog_seconds is None):
        parser.error("--watch-executable and --watchdog-seconds must be used together")
    if args.watchdog_seconds is not None and args.watchdog_seconds <= 0:
        parser.error("--watchdog-seconds must be positive")
    try:
        return run_on_private_desktop(
            command,
            Path.cwd(),
            watch_executable=args.watch_executable,
            watchdog_seconds=args.watchdog_seconds,
        )
    except (OSError, RuntimeError, ValueError) as exc:
        print(f"private-desktop launcher: {exc}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
