import java.util.HashSet;
import java.util.Set;

import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.address.AddressSet;
import ghidra.program.model.lang.Register;
import ghidra.program.model.listing.Instruction;
import ghidra.program.model.listing.InstructionIterator;
import ghidra.program.model.mem.Memory;
import ghidra.program.model.pcode.PcodeOp;
import ghidra.program.model.pcode.Varnode;

/** Read-only corpus validation for the fixed MCS-251 processor language. */
public class ValidatePlextor80251Language extends GhidraScript {
    private static final String LANGUAGE = "80251-fixed:BE:24:default";
    private static final long CODE_START = 0xf80050;
    private static final long CODE_END = 0xfef193;

    private void require(boolean condition, String message) {
        if (!condition) {
            throw new AssertionError(message);
        }
    }

    private AddressSet codeRange() {
        return new AddressSet(toAddr(CODE_START), toAddr(CODE_END));
    }

    private void requireInstruction(
            long offset, String rendered, int length) {
        Address address = toAddr(offset);
        Instruction instruction =
            currentProgram.getListing().getInstructionAt(address);
        require(instruction != null && instruction.getLength() == length &&
                rendered.equals(instruction.toString()),
            "Expected " + rendered + " at " + address + ", found " +
            instruction);
    }

    private int countPswWrites(Instruction instruction) {
        int writes = 0;
        for (PcodeOp operation : instruction.getPcode()) {
            Varnode output = operation.getOutput();
            if (output != null && output.isRegister() &&
                    output.getAddress().getOffset() == 0x48 &&
                    output.getSize() == 1) {
                writes++;
            }
        }
        return writes;
    }

    private void validateDecoderProbes() {
        requireInstruction(0xf80050, "EJMP 0xfdcea9", 4);
        requireInstruction(0xfd8fe6, "CPL A", 1);
        requireInstruction(0xfd9013, "CPL A", 1);
        requireInstruction(0xfd9028, "CPL CY", 1);
        requireInstruction(0xfd9029, "ERET", 1);
        requireInstruction(0xfee4b4, "MOV @WR4+0x2,WR2", 4);
        requireInstruction(0xfd6c36, "MOV DR4,0x000b0c", 4);
        requireInstruction(0xfd6c3a, "ADD DR4,#0x96", 4);
        requireInstruction(0xfd6c3e, "MOV DR0,0x000b10", 4);
        requireInstruction(0xfd6c42, "ADD DR0,DR4", 2);
        requireInstruction(0xfd6daf, "CMP DR12,#0x4b", 4);
        requireInstruction(0xfd6db5, "CMP DR12,#0xfffffc18", 4);
        requireInstruction(0xfd6dbb, "CMP DR12,#0xffffb1e0", 4);
        requireInstruction(0xf92a80, "SUB DR0,DR0", 2);
        requireInstruction(0xf92a82, "XRL WR6,WR6", 2);
        requireInstruction(0xf92a84, "MOV WR4,#0x1000", 4);
        requireInstruction(0xf92a88, "ECALL 0xfdccc1", 4);
        requireInstruction(0xf9130a, "JNB 0x0013,0xf91310", 3);
        requireInstruction(0xf9130d, "LJMP 0xf913a4", 3);
        println("DECODER_PROBES=PASS");
    }

    private void validateExtendedBitSpaces() throws Exception {
        Set<Integer> bitOperations = Set.of(
            0x1, 0x2, 0x3, 0x7, 0x8, 0x9,
            0xa, 0xb, 0xc, 0xd, 0xe, 0xf);
        Set<Integer> registerBackedSfrs = Set.of(0xd0, 0xe0, 0xf0);
        Memory memory = currentProgram.getMemory();
        int checked = 0;
        InstructionIterator instructions =
            currentProgram.getListing().getInstructions(codeRange(), true);
        while (instructions.hasNext()) {
            Instruction instruction = instructions.next();
            Address address = instruction.getAddress();
            if ((memory.getByte(address) & 0xff) != 0xa9 ||
                    instruction.getLength() < 3) {
                continue;
            }
            int operation = (memory.getByte(address.add(1)) >> 4) & 0xf;
            if (!bitOperations.contains(operation)) {
                continue;
            }
            int byteAddress = memory.getByte(address.add(2)) & 0xff;
            String expectedSpace = byteAddress < 0x80 ? "RAM" : "SFR";
            String wrongSpace = byteAddress < 0x80 ? "SFR" : "RAM";
            boolean sawExpected = false;
            boolean sawWrong = false;
            for (PcodeOp pcode : instruction.getPcode()) {
                for (Varnode input : pcode.getInputs()) {
                    if (input.getAddress().getOffset() == byteAddress) {
                        String space =
                            input.getAddress().getAddressSpace().getName();
                        sawExpected |= expectedSpace.equals(space);
                        sawWrong |= wrongSpace.equals(space);
                    }
                }
                Varnode output = pcode.getOutput();
                if (output != null &&
                        output.getAddress().getOffset() == byteAddress) {
                    String space =
                        output.getAddress().getAddressSpace().getName();
                    sawExpected |= expectedSpace.equals(space);
                    sawWrong |= wrongSpace.equals(space);
                }
            }
            require(!sawWrong && (sawExpected ||
                    registerBackedSfrs.contains(byteAddress)),
                "Extended-bit byte-space regression at " + address +
                " for 0x" + Integer.toHexString(byteAddress));
            checked++;
        }
        require(checked >= 10,
            "Too few extended-bit instructions validated: " + checked);
        println("EXTENDED_BIT_BYTE_SPACES=" + checked);
    }

    private void validateLegacyIncrementDecrementFlags() {
        int checked = 0;
        InstructionIterator instructions =
            currentProgram.getListing().getInstructions(codeRange(), true);
        while (instructions.hasNext()) {
            Instruction instruction = instructions.next();
            String mnemonic = instruction.getMnemonicString();
            if (("INC".equals(mnemonic) || "DEC".equals(mnemonic)) &&
                    instruction.getNumOperands() == 1) {
                require(countPswWrites(instruction) == 2,
                    instruction + " does not write exactly N/Z at " +
                    instruction.getAddress());
                checked++;
            }
        }
        require(checked == 1710,
            "Unexpected legacy INC/DEC corpus: " + checked);
        println("LEGACY_INC_DEC_FLAG_WRITES=" + checked);
    }

    private void validateArithmeticFlags() {
        int arithmetic = 0;
        int multiplyDivide = 0;
        InstructionIterator instructions =
            currentProgram.getListing().getInstructions(codeRange(), true);
        while (instructions.hasNext()) {
            Instruction instruction = instructions.next();
            String mnemonic = instruction.getMnemonicString();
            if ("ADD".equals(mnemonic) || "ADDC".equals(mnemonic) ||
                    "SUB".equals(mnemonic) || "SUBB".equals(mnemonic) ||
                    "CMP".equals(mnemonic)) {
                Register destination = instruction.getRegister(0);
                int expected = destination != null &&
                    destination.getBitLength() <= 8 ? 5 : 4;
                require(countPswWrites(instruction) >= expected,
                    instruction + " lacks arithmetic PSW writes at " +
                    instruction.getAddress());
                arithmetic++;
            }
            else if ("MUL".equals(mnemonic) || "DIV".equals(mnemonic)) {
                require(countPswWrites(instruction) >= 4,
                    instruction + " lacks MUL/DIV PSW writes at " +
                    instruction.getAddress());
                multiplyDivide++;
            }
        }
        require(arithmetic == 12758 && multiplyDivide == 1778,
            "Unexpected arithmetic corpus: arithmetic=" + arithmetic +
            " multiplyDivide=" + multiplyDivide);
        println("ARITHMETIC_FLAG_WRITES=" + arithmetic +
            " MUL_DIV_FLAG_WRITES=" + multiplyDivide);
    }

    private void validateByteMultiplyDividePairs() {
        int checked = 0;
        int r5Destinations = 0;
        InstructionIterator instructions =
            currentProgram.getListing().getInstructions(codeRange(), true);
        while (instructions.hasNext()) {
            Instruction instruction = instructions.next();
            String mnemonic = instruction.getMnemonicString();
            if (!("MUL".equals(mnemonic) || "DIV".equals(mnemonic)) ||
                    instruction.getNumOperands() != 2) {
                continue;
            }
            Register destination = instruction.getRegister(0);
            if (destination == null ||
                    !destination.getName().matches("R(?:[0-9]|1[0-5])")) {
                continue;
            }
            int destinationNumber = Integer.parseInt(
                destination.getName().substring(1));
            int expectedHigh = destinationNumber & ~1;
            int expectedLow = expectedHigh + 1;
            Set<Integer> written = new HashSet<>();
            for (PcodeOp pcode : instruction.getPcode()) {
                Varnode output = pcode.getOutput();
                if (output != null && output.isRegister() &&
                        output.getSize() == 1 &&
                        output.getAddress().getOffset() >= 0 &&
                        output.getAddress().getOffset() <= 0xf) {
                    written.add((int)output.getAddress().getOffset());
                }
            }
            require(written.equals(Set.of(expectedHigh, expectedLow)),
                "Wrong byte MUL/DIV destination pair at " +
                instruction.getAddress() + ": " + written);
            if (destinationNumber == 5) {
                r5Destinations++;
            }
            checked++;
        }
        require(checked == 552 && r5Destinations == 79,
            "Unexpected byte MUL/DIV corpus: checked=" + checked +
            " R5=" + r5Destinations);
        println("BYTE_MUL_DIV_REGISTER_PAIRS=" + checked +
            " R5_DESTINATIONS=" + r5Destinations);
    }

    private void validateExplicitStackWidths() {
        int wordOperations = 0;
        int dwordOperations = 0;
        InstructionIterator instructions =
            currentProgram.getListing().getInstructions(codeRange(), true);
        while (instructions.hasNext()) {
            Instruction instruction = instructions.next();
            String mnemonic = instruction.getMnemonicString();
            if (!("PUSH".equals(mnemonic) || "POP".equals(mnemonic))) {
                continue;
            }
            Register operand = instruction.getRegister(0);
            if (operand == null || (operand.getBitLength() != 16 &&
                    operand.getBitLength() != 32)) {
                continue;
            }
            int expectedBytes = operand.getBitLength() / 8;
            boolean sawExpectedAccess = false;
            for (PcodeOp pcode : instruction.getPcode()) {
                if ("PUSH".equals(mnemonic) &&
                        pcode.getOpcode() == PcodeOp.STORE &&
                        pcode.getNumInputs() == 3 &&
                        pcode.getInput(2).getSize() == expectedBytes) {
                    sawExpectedAccess = true;
                }
                if ("POP".equals(mnemonic) &&
                        pcode.getOpcode() == PcodeOp.LOAD &&
                        pcode.getOutput() != null &&
                        pcode.getOutput().getSize() == expectedBytes) {
                    sawExpectedAccess = true;
                }
            }
            require(sawExpectedAccess,
                instruction + " lacks a " + expectedBytes +
                "-byte explicit stack access at " +
                instruction.getAddress());
            if (expectedBytes == 2) {
                wordOperations++;
            }
            else {
                dwordOperations++;
            }
        }
        require(wordOperations == 588 && dwordOperations == 909,
            "Unexpected explicit-stack corpus: word=" + wordOperations +
            " dword=" + dwordOperations);
        println("EXPLICIT_STACK_WIDTHS_WORD=" + wordOperations +
            " DWORD=" + dwordOperations);
    }

    @Override
    protected void run() throws Exception {
        require(LANGUAGE.equals(currentProgram.getLanguageID().toString()),
            "Unexpected processor language " + currentProgram.getLanguageID());
        require(currentProgram.getMemory().getBlock(toAddr(CODE_START)) != null,
            "Firmware code range is not mapped");
        validateDecoderProbes();
        validateExtendedBitSpaces();
        validateLegacyIncrementDecrementFlags();
        validateArithmeticFlags();
        validateByteMultiplyDividePairs();
        validateExplicitStackWidths();
        println("PROCESSOR_LANGUAGE_VALIDATION=PASS");
    }
}
