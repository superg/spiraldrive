import java.util.HashSet;
import java.util.Locale;
import java.util.Set;

import ghidra.app.script.GhidraScript;
import ghidra.framework.model.DomainFile;
import ghidra.framework.model.DomainFolder;
import ghidra.framework.options.Options;
import ghidra.program.model.address.Address;
import ghidra.program.model.address.AddressSet;
import ghidra.program.model.address.AddressSpace;
import ghidra.program.model.address.AddressSetView;
import ghidra.program.model.lang.Register;
import ghidra.program.model.listing.DataIterator;
import ghidra.program.model.listing.Data;
import ghidra.program.model.listing.Function;
import ghidra.program.model.listing.FunctionIterator;
import ghidra.program.model.listing.FunctionManager;
import ghidra.program.model.listing.FunctionTag;
import ghidra.program.model.listing.Instruction;
import ghidra.program.model.listing.InstructionIterator;
import ghidra.program.model.mem.Memory;
import ghidra.program.model.mem.MemoryBlock;
import ghidra.program.model.pcode.PcodeOp;
import ghidra.program.model.pcode.Varnode;
import ghidra.program.model.symbol.SourceType;
import ghidra.program.model.symbol.Reference;
import ghidra.program.model.symbol.Symbol;

/**
 * Compact, read-only validation for the canonical PX-W5224A Ghidra programs.
 *
 * This script validates intrinsic database invariants without replaying the
 * address-by-address analysis metadata whose authoritative copy is the Ghidra
 * project itself.  It deliberately does not create, rename, type, or repair
 * anything.
 */
public class ValidatePlextorCanonicalProject extends GhidraScript {
    private static final String LANGUAGE = "80251-fixed:BE:24:default";
    private static final String MAIN_ROLE = "PX-W5224A main firmware";
    private static final String RUNTIME_ROLE =
        "PX-W5224A runtime recovery capture";
    private static final String MAIN_SHA256 =
        "622e1abd99894e913300f3b737d0cb82eb217061f342d26536b88125769e50b2";
    private static final String RUNTIME_SHA256 =
        "7ad6513ecb74f6107b3abdbdbb59b29b00334adb79cbf48ba1322d0442a42e80";
    private boolean preparedBaseline;

    private Options programInformation() {
        return currentProgram.getOptions("Program Information");
    }

    private void require(boolean condition, String message) {
        if (!condition) {
            throw new AssertionError(message);
        }
    }

    private void requireOption(String name, String expected) {
        String actual = programInformation().getString(name, null);
        require(expected.equals(actual),
            "Unexpected Program Information option " + name +
            ": expected " + expected + ", found " + actual);
    }

    private void requireIntOption(String name, int expected) {
        int actual = programInformation().getInt(name, Integer.MIN_VALUE);
        require(actual == expected,
            "Unexpected Program Information option " + name +
            ": expected " + expected + ", found " + actual);
    }

    private Address address(String spaceName, long offset) {
        AddressSpace space =
            currentProgram.getAddressFactory().getAddressSpace(spaceName);
        require(space != null, "Missing address space " + spaceName);
        return space.getAddress(offset);
    }

    private MemoryBlock requireBlock(
            String name, String spaceName, long startOffset, long endOffset,
            boolean initialized, boolean overlay) {
        MemoryBlock block = currentProgram.getMemory().getBlock(name);
        require(block != null, "Missing memory block " + name);
        require(block.getStart().equals(address(spaceName, startOffset)) &&
                block.getEnd().equals(address(spaceName, endOffset)),
            "Unexpected range for memory block " + name + ": " +
            block.getStart() + ".." + block.getEnd());
        require(block.isInitialized() == initialized,
            "Unexpected initialized state for memory block " + name);
        require(block.getStart().getAddressSpace().isOverlaySpace() == overlay,
            "Unexpected overlay state for memory block " + name);
        return block;
    }

    private int tagCount(Function function, String prefix) {
        int count = 0;
        for (FunctionTag tag : function.getTags()) {
            if (tag.getName().startsWith(prefix)) {
                count++;
            }
        }
        return count;
    }

    private boolean hasTag(Function function, String name) {
        for (FunctionTag tag : function.getTags()) {
            if (name.equals(tag.getName())) {
                return true;
            }
        }
        return false;
    }

    private void requireCommonIdentity(
            String role, String sourcePath, String sha256, int functions) {
        require(LANGUAGE.equals(currentProgram.getLanguageID().toString()),
            "Unexpected processor language " + currentProgram.getLanguageID());
        requireOption("Plextor.Role", role);
        requireOption("Plextor.HardwareModel", "PLEXTOR CD-R PX-W5224A");
        requireOption("Plextor.SourcePath", sourcePath);
        requireOption("Plextor.SourceSHA256", sha256);
        requireOption("Executable SHA256", sha256);
        requireOption("Plextor.ProcessorLanguageRelease",
            "80251-fixed:BE:24:default v1.3");
        requireIntOption("Plextor.ValidatedFunctionCount", functions);
        require(currentProgram.getFunctionManager().getFunctionCount() ==
                functions,
            "Unexpected aggregate function count");
    }

    private void requireCanonicalProjectLayout() throws Exception {
        DomainFolder root = state.getProject().getProjectData().getRootFolder();
        DomainFolder[] rootFolders = root.getFolders();
        require(root.getFiles().length == 0,
            "Canonical project root contains unexpected files");
        require(rootFolders.length == 1 &&
                "validated".equals(rootFolders[0].getName()),
            "Canonical project must contain only the /validated folder");

        DomainFolder validated = rootFolders[0];
        require(validated.getFolders().length == 0,
            "/validated contains unexpected subfolders");
        Set<String> expected = new HashSet<>();
        expected.add("MONT_104.bin");
        expected.add("read_memory_ff0000.debug");
        DomainFile[] files = validated.getFiles();
        require(files.length == expected.size(),
            "/validated must contain exactly two programs");
        for (DomainFile file : files) {
            require(expected.remove(file.getName()),
                "Unexpected /validated entry " + file.getName());
        }
        require(expected.isEmpty(),
            "Missing canonical /validated program(s): " + expected);
    }

    private long[] validateInstructions(
            AddressSetView range, AddressSetView functionBodies) {
        FunctionManager functions = currentProgram.getFunctionManager();
        long count = 0;
        long bytes = 0;
        InstructionIterator instructions =
            currentProgram.getListing().getInstructions(range, true);
        while (instructions.hasNext()) {
            Instruction instruction = instructions.next();
            count++;
            bytes += instruction.getLength();
            require(functionBodies.contains(instruction.getAddress()) &&
                    functionBodies.contains(instruction.getMaxAddress()),
                "Instruction outside a function at " +
                instruction.getAddress());
            if (instruction.getFlowType().isCall()) {
                for (Address target : instruction.getFlows()) {
                    if (range.contains(target) &&
                            currentProgram.getListing().getInstructionAt(
                                target) != null) {
                        require(functions.getFunctionAt(target) != null,
                            "Direct call target is not a function at " +
                            target + " from " + instruction.getAddress());
                    }
                }
            }
        }
        return new long[] { count, bytes };
    }

    private long definedDataBytes(AddressSetView range) {
        long bytes = 0;
        DataIterator data =
            currentProgram.getListing().getDefinedData(range, true);
        while (data.hasNext()) {
            bytes += data.next().getLength();
        }
        return bytes;
    }

    private Address pointerAt(Address entry) throws Exception {
        Memory memory = currentProgram.getMemory();
        long value = ((memory.getByte(entry.add(1)) & 0xffL) << 16) |
            ((memory.getByte(entry.add(2)) & 0xffL) << 8) |
            (memory.getByte(entry.add(3)) & 0xffL);
        return address("RAM", value);
    }

    private void requireFunctionPointerTable(long offset, int entries)
            throws Exception {
        Address base = address("RAM", offset);
        for (int index = 0; index < entries; index++) {
            Address target = pointerAt(base.add(index * 4L));
            require(currentProgram.getFunctionManager()
                    .getFunctionAt(target) != null,
                "Function-pointer table entry " + index + " at " + base +
                " does not resolve to a function: " + target);
        }
    }

    private void requireDataPointerTable(long offset, int entries)
            throws Exception {
        Address base = address("RAM", offset);
        Data table = currentProgram.getListing().getDefinedDataAt(base);
        require(table != null && table.getLength() == entries * 4,
            "Missing " + entries + "-entry data-pointer table at " + base);
        for (int index = 0; index < entries; index++) {
            Address entry = base.add(index * 4L);
            Address target = pointerAt(entry);
            boolean found = false;
            for (Reference reference : currentProgram.getReferenceManager()
                    .getReferencesFrom(entry)) {
                if (reference.getReferenceType().isData() &&
                        reference.getToAddress().equals(target)) {
                    found = true;
                    break;
                }
            }
            require(found, "Missing data reference from " + entry +
                " to " + target);
        }
    }

    private Set<String> preparationRiskReasons(Function function) {
        Set<String> reasons = new HashSet<>();
        if (function.getBody().getNumAddresses() >= 100) {
            reasons.add("body>=100");
        }
        if (function.getCallingFunctions(monitor).size() > 3) {
            reasons.add("callers>3");
        }
        InstructionIterator instructions = currentProgram.getListing()
            .getInstructions(function.getBody(), true);
        while (instructions.hasNext()) {
            Instruction instruction = instructions.next();
            if (instruction.toString().toUpperCase(Locale.ROOT)
                    .contains("SPX")) {
                reasons.add("explicit_stack_use");
            }
            for (Reference reference : currentProgram.getReferenceManager()
                    .getReferencesFrom(instruction.getAddress())) {
                if (!reference.getReferenceType().isData()) {
                    continue;
                }
                Address target = reference.getToAddress();
                String space = target.getAddressSpace().getName();
                if ("SFR".equals(space)) {
                    reasons.add("sfr");
                }
                else if ("STACK".equalsIgnoreCase(space)) {
                    reasons.add("explicit_stack_use");
                }
                else if ("RAM".equals(space)) {
                    long offset = target.getOffset();
                    if (offset >= 0x18000 && offset <= 0x180ff) {
                        reasons.add("controller_mmio");
                    }
                    else if (offset > 0x180ff && offset < 0xf80000) {
                        reasons.add("external_memory");
                    }
                }
            }
        }
        return reasons;
    }

    private long[] preparedReferenceMetrics(AddressSetView bodies) {
        long references = 0;
        Set<Address> targets = new HashSet<>();
        InstructionIterator instructions = currentProgram.getListing()
            .getInstructions(bodies, true);
        while (instructions.hasNext()) {
            Instruction instruction = instructions.next();
            for (Reference reference : currentProgram.getReferenceManager()
                    .getReferencesFrom(instruction.getAddress())) {
                if (!reference.getReferenceType().isData()) {
                    continue;
                }
                references++;
                targets.add(reference.getToAddress());
            }
        }
        return new long[] { references, targets.size() };
    }

    private void requireMainChecksum(Address start) throws Exception {
        Memory memory = currentProgram.getMemory();
        long checksum = 0x8000;
        byte[] buffer = new byte[4096];
        long remaining = 0x79ffe;
        Address cursor = start;
        while (remaining != 0) {
            int requested = (int)Math.min(remaining, buffer.length);
            int actual = memory.getBytes(cursor, buffer, 0, requested);
            require(actual == requested,
                "Unable to read firmware bytes at " + cursor);
            for (int index = 0; index < actual; index++) {
                checksum = (checksum + (buffer[index] & 0xff)) & 0xffff;
            }
            remaining -= actual;
            cursor = cursor.add(actual);
        }
        int stored = ((memory.getByte(start.add(0x79ffe)) & 0xff) << 8) |
            (memory.getByte(start.add(0x79fff)) & 0xff);
        require(stored == checksum,
            "Firmware checksum mismatch: stored 0x" +
            Integer.toHexString(stored) + ", computed 0x" +
            Long.toHexString(checksum));
    }

    private void validateMain() throws Exception {
        requireCommonIdentity(
            MAIN_ROLE, "data/W5224A104/MONT_104.bin", MAIN_SHA256, 1717);
        requireOption("Plextor.FirmwareVersion", "1.04");
        requireOption("Plextor.MappedImageRange",
            "RAM:f80000..RAM:ffffff");
        require(currentProgram.getMemory().getBlocks().length == 10,
            "Expected exactly ten main-firmware memory blocks");
        requireBlock("REG_BANK_1", "RAM", 0x000000, 0x000007,
            false, false);
        requireBlock("REG_BANK_2", "RAM", 0x000008, 0x00000f,
            false, false);
        requireBlock("REG_BANK_3", "RAM", 0x000010, 0x000017,
            false, false);
        requireBlock("REG_BANK_4", "RAM", 0x000018, 0x00001f,
            false, false);
        requireBlock("INTMEM", "RAM", 0x000020, 0x0000ff,
            false, false);
        requireBlock("ExternalRAM_MMIO", "RAM", 0x000100, 0xf7ffff,
            false, false);
        requireBlock("RAM", "RAM", 0xf80000, 0xffffff,
            true, false);
        requireBlock("SFR", "SFR", 0x80, 0xff, false, false);
        requireBlock("BITS", "BITS", 0x100, 0x3ff, false, false);
        requireBlock("SFR-BITS", "BITS", 0x400, 0x7ff,
            false, false);
        requireDataPointerTable(0xf80200, 208);
        requireFunctionPointerTable(0xf83ef7, 256);
        requireFunctionPointerTable(0xff9383, 32);
        requireFunctionPointerTable(0xf8489f, 10);
        requireFunctionPointerTable(0xff9617, 7);

        FunctionManager manager = currentProgram.getFunctionManager();
        FunctionIterator functions = manager.getFunctions(true);
        AddressSet bodies = new AddressSet();
        Set<Address> customImplementations = new HashSet<>();
        long count = 0, defaultNames = 0, named = 0, thunks = 0;
        long callEdges = 0;
        long customRecords = 0, reviewed = 0, candidates = 0;
        long unsafeCandidates = 0, unsafeAddressNames = 0;
        while (functions.hasNext()) {
            Function function = functions.next();
            count++;
            callEdges += function.getCalledFunctions(monitor).size();
            bodies.add(function.getBody());
            if (function.getSymbol().getSource() == SourceType.DEFAULT) {
                defaultNames++;
            }
            else {
                named++;
            }
            if (function.isThunk()) {
                thunks++;
            }
            if (function.hasCustomVariableStorage()) {
                customRecords++;
                Function implementation = function.isThunk()
                    ? function.getThunkedFunction(true) : function;
                require(implementation != null,
                    "Custom-storage thunk has no implementation at " +
                    function.getEntryPoint());
                customImplementations.add(implementation.getEntryPoint());
            }
            require(tagCount(function, "unit:") == 1,
                "Function lacks one unit tag at " + function.getEntryPoint());
            require(tagCount(function, "placement:") == 1,
                "Function lacks one placement tag at " +
                function.getEntryPoint());
            require(tagCount(function, "semantic:") == 1,
                "Function lacks one semantic tag at " +
                function.getEntryPoint());
            require(tagCount(function, "review:") == 1,
                "Function lacks one review tag at " + function.getEntryPoint());
            if (hasTag(function, "placement:reviewed")) {
                reviewed++;
            }
            if (hasTag(function, "placement:candidate")) {
                candidates++;
            }
            if (preparedBaseline &&
                    (hasTag(function, "placement:candidate") ||
                        function.getName().startsWith("FUN_"))) {
                Set<String> reasons = preparationRiskReasons(function);
                if (!reasons.isEmpty()) {
                    printerr("PREPARATION_RISK " +
                        function.getEntryPoint() + " " + function.getName() +
                        " " + reasons);
                    if (hasTag(function, "placement:candidate")) {
                        unsafeCandidates++;
                    }
                    if (function.getName().startsWith("FUN_")) {
                        unsafeAddressNames++;
                    }
                }
            }
        }
        require(!manager.getExternalFunctions().hasNext(),
            "Main firmware unexpectedly contains external functions");
        require(count == 1717 && named + defaultNames == count && thunks == 23,
            "Unexpected main function/name/thunk metrics: functions=" + count +
            " named=" + named + " default=" + defaultNames +
            " thunks=" + thunks);
        require(reviewed + candidates == count,
                "Unexpected subsystem review totals: reviewed=" + reviewed +
                " candidates=" + candidates);
        if (preparedBaseline) {
            require(named == 1579 && defaultNames == 138,
                "Prepared-baseline name totals changed: named=" + named +
                " default=" + defaultNames);
            require(customRecords == 1364 &&
                    customImplementations.size() == 1350,
                "Prepared-baseline custom-storage metrics changed: records=" +
                customRecords + " implementations=" +
                customImplementations.size());
            require(reviewed == 1596 && candidates == 121,
                "Prepared-baseline review totals changed: reviewed=" +
                reviewed + " candidates=" + candidates);
            require(callEdges == 6031,
                "Prepared-baseline call-edge count changed: " + callEdges);
            require(unsafeCandidates == 0 && unsafeAddressNames == 0,
                "Prepared-baseline risk tail reopened: candidates=" +
                unsafeCandidates + " address_names=" + unsafeAddressNames);
        }
        require(bodies.getNumAddresses() == 430777,
            "Unexpected function-body coverage: " +
            bodies.getNumAddresses());

        Address start = address("RAM", 0xf80000);
        AddressSet image = new AddressSet(start, address("RAM", 0xffffff));
        long[] instructions = validateInstructions(image, bodies);
        long dataBytes = definedDataBytes(image);
        require(instructions[0] == 149194 && instructions[1] == 430777,
            "Unexpected instruction metrics: count=" + instructions[0] +
            " bytes=" + instructions[1]);
        require(dataBytes == 93511,
            "Unexpected defined-data coverage: " + dataBytes);
        require(instructions[1] + dataBytes == 0x80000,
            "Firmware image is not completely classified");
        if (preparedBaseline) {
            long allDataItems = 0;
            DataIterator allData = currentProgram.getListing()
                .getDefinedData(true);
            while (allData.hasNext()) {
                allData.next();
                allDataItems++;
            }
            long imageDataItems = 0;
            DataIterator imageData = currentProgram.getListing()
                .getDefinedData(image, true);
            while (imageData.hasNext()) {
                imageData.next();
                imageDataItems++;
            }
            require(allDataItems == 1691,
                "Prepared-baseline data-item count changed: " +
                allDataItems);
            require(imageDataItems == 102,
                "Prepared-baseline image-data count changed: " +
                imageDataItems);
            long[] referenceMetrics = preparedReferenceMetrics(bodies);
            require(referenceMetrics[0] == 33645 &&
                    referenceMetrics[1] == 2866,
                "Prepared-baseline data-reference metrics changed: refs=" +
                referenceMetrics[0] + " targets=" + referenceMetrics[1]);
            println("MAIN_GRAPH=6031_call_edges+" + referenceMetrics[0] +
                "_data_refs+" + referenceMetrics[1] + "_targets");
            println("MAIN_PREPARATION_RISK=0_candidate+0_address_named");
        }
        requireMainChecksum(start);

        println("ROLE=main FUNCTIONS=1717 INSTRUCTIONS=149194 " +
            "CODE_BYTES=430777 DATA_BYTES=93511");
        println("MAIN_NAMES=" + named + "_named+" + defaultNames +
            "_default THUNKS=" + thunks);
        println("MAIN_CUSTOM_STORAGE=" + customRecords + "_records+" +
            customImplementations.size() + "_implementations");
        println("MAIN_TAGS=" + reviewed + "_reviewed+" + candidates +
            "_candidate");
    }

    private void requireOverlayCopy(
            String name, long sourceOffset, long endOffset) throws Exception {
        Memory memory = currentProgram.getMemory();
        MemoryBlock block = requireBlock(
            name, name, 0x800, endOffset, true, true);
        int length = (int)block.getSize();
        byte[] source = new byte[length];
        byte[] relocated = new byte[length];
        require(memory.getBytes(address("RAM", sourceOffset), source) == length,
            "Unable to read overlay source for " + name);
        require(memory.getBytes(block.getStart(), relocated) == length,
            "Unable to read overlay bytes for " + name);
        for (int index = 0; index < length; index++) {
            require(source[index] == relocated[index],
                "Relocated byte mismatch in " + name + " at +0x" +
                Integer.toHexString(index));
        }
    }

    private void requireOverlayFunction(
            String spaceName, long endOffset, String expectedName)
            throws Exception {
        Address start = address(spaceName, 0x800);
        Address end = address(spaceName, endOffset);
        require(start.getAddressSpace().isOverlaySpace(),
            "Expected overlay address space " + spaceName);
        Function function = currentProgram.getFunctionManager()
            .getFunctionAt(start);
        require(function != null && expectedName.equals(function.getName()),
            "Missing " + expectedName + " at " + start);
        require(function.getBody().getNumAddressRanges() == 1 &&
                function.getBody().contains(start, end) &&
                function.getBody().getNumAddresses() ==
                    end.subtract(start) + 1L,
            expectedName + " does not have one complete contiguous body: " +
            function.getBody());

        int instructionCount = 0;
        Address cursor = start;
        while (cursor.compareTo(end) <= 0) {
            Instruction instruction = currentProgram.getListing()
                .getInstructionAt(cursor);
            require(instruction != null &&
                    function.getBody().contains(instruction.getMaxAddress()),
                "Missing instruction at " + cursor + " in " + expectedName);
            instructionCount++;
            cursor = instruction.getMaxAddress().next();
        }
        println(expectedName + "_INSTRUCTION_COUNT=" + instructionCount);
    }

    private void requireInstruction(
            String spaceName, long offset, String expected, int length) {
        Address instructionAddress = address(spaceName, offset);
        Instruction instruction = currentProgram.getListing()
            .getInstructionAt(instructionAddress);
        require(instruction != null && instruction.getLength() == length &&
                expected.equals(instruction.toString()),
            "Expected " + expected + " at " + instructionAddress +
            ", found " + instruction);
    }

    private void requireRamInstruction(
            long offset, String expected, int length) {
        requireInstruction("RAM", offset, expected, length);
    }

    private void requireFunctionNamed(long offset, String expectedName) {
        Address entry = address("RAM", offset);
        Function function = currentProgram.getFunctionManager()
            .getFunctionAt(entry);
        require(function != null && expectedName.equals(function.getName()),
            "Expected function " + expectedName + " at " + entry +
            ", found " + function);
    }

    private void requireRamBytes(long offset, int... expected)
            throws Exception {
        Address start = address("RAM", offset);
        for (int index = 0; index < expected.length; index++) {
            int actual = currentProgram.getMemory()
                .getByte(start.add(index)) & 0xff;
            require(actual == expected[index],
                "Expected byte 0x" + Integer.toHexString(expected[index]) +
                " at " + start.add(index) + ", found 0x" +
                Integer.toHexString(actual));
        }
    }

    private void requireFlow(long fromOffset, long toOffset) {
        Address from = address("RAM", fromOffset);
        Address to = address("RAM", toOffset);
        Instruction instruction = currentProgram.getListing()
            .getInstructionAt(from);
        require(instruction != null, "Missing flow instruction at " + from);
        for (Address flow : instruction.getFlows()) {
            if (flow.equals(to)) {
                return;
            }
        }
        require(false, "Expected flow from " + from + " to " + to +
            ", found " + instruction);
    }

    private void requireContiguousRamFunction(
            long offset, int byteCount, int instructionCount) {
        Address start = address("RAM", offset);
        Address end = start.add(byteCount - 1L);
        Function function = currentProgram.getFunctionManager()
            .getFunctionAt(start);
        require(function != null &&
                function.getBody().getNumAddressRanges() == 1 &&
                function.getBody().getNumAddresses() == byteCount &&
                function.getBody().contains(start, end),
            "Expected one contiguous " + byteCount +
            "-byte function at " + start);
        int actualInstructions = 0;
        Address cursor = start;
        while (cursor.compareTo(end) <= 0) {
            Instruction instruction = currentProgram.getListing()
                .getInstructionAt(cursor);
            require(instruction != null &&
                    instruction.getMaxAddress().compareTo(end) <= 0,
                "Missing bounded instruction at " + cursor);
            actualInstructions++;
            cursor = instruction.getMaxAddress().next();
        }
        require(actualInstructions == instructionCount,
            "Expected " + instructionCount + " instructions at " + start +
            ", found " + actualInstructions);
    }

    private void validateRuntimeIncrementDecrementFlags() {
        int checked = 0;
        InstructionIterator instructions = currentProgram.getListing()
            .getInstructions(true);
        while (instructions.hasNext()) {
            Instruction instruction = instructions.next();
            String mnemonic = instruction.getMnemonicString();
            if (!("INC".equals(mnemonic) || "DEC".equals(mnemonic)) ||
                    instruction.getNumOperands() != 1) {
                continue;
            }
            int pswWrites = 0;
            for (PcodeOp operation : instruction.getPcode()) {
                Varnode output = operation.getOutput();
                if (output != null && output.isRegister() &&
                        output.getAddress().getOffset() == 0x48 &&
                        output.getSize() == 1) {
                    pswWrites++;
                }
            }
            require(pswWrites == 2,
                instruction + " has " + pswWrites +
                " PSW writes instead of exactly N/Z at " +
                instruction.getAddress());
            checked++;
        }
        require(checked > 0,
            "No runtime legacy INC/DEC instructions were validated");
        println("RUNTIME_LEGACY_INC_DEC_FLAG_WRITES=" + checked);
    }

    private void validateRuntimeByteMultiplyDividePairs() {
        int checked = 0;
        int r5Destinations = 0;
        InstructionIterator instructions = currentProgram.getListing()
            .getInstructions(true);
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
            for (PcodeOp operation : instruction.getPcode()) {
                Varnode output = operation.getOutput();
                if (output != null && output.isRegister() &&
                        output.getSize() == 1 &&
                        output.getAddress().getOffset() >= 0 &&
                        output.getAddress().getOffset() <= 0xf) {
                    written.add((int)output.getAddress().getOffset());
                }
            }
            require(written.equals(Set.of(expectedHigh, expectedLow)),
                "Wrong byte MUL/DIV destination pair at " +
                instruction.getAddress() + ": " + written +
                ", expected R" + expectedHigh + ":R" + expectedLow);
            if (destinationNumber == 5) {
                r5Destinations++;
            }
            checked++;
        }
        require(checked > 0,
            "No runtime byte MUL/DIV register pairs were validated");
        println("RUNTIME_BYTE_MUL_DIV_REGISTER_PAIRS=" + checked +
            " R5_DESTINATIONS=" + r5Destinations);
    }

    private void validateRuntimeInterruptRouting() throws Exception {
        Symbol selector = currentProgram.getSymbolTable().getPrimarySymbol(
            address("RAM", 0x0000fc));
        require(selector != null &&
                "MainFirmwareInterruptVectorSelect".equals(
                    selector.getName()),
            "Missing MainFirmwareInterruptVectorSelect at RAM:0000fc");
        Data selectorData = currentProgram.getListing().getDefinedDataAt(
            address("RAM", 0x0000fc));
        require(selectorData != null && selectorData.getLength() == 1,
            "Interrupt vector selector is not a one-byte data object");

        long[] routers = {
            0xff3f84L, 0xff3f9bL, 0xff3fb2L, 0xff3fc9L, 0xff3fe0L
        };
        long[] vectors = {
            0xff0003L, 0xff000bL, 0xff0013L, 0xff0033L, 0xff004bL
        };
        String[] names = {
            "RouteControllerEventInterrupt",
            "RouteHostInterfaceInterrupt",
            "RouteControllerPacketInterrupt",
            "RouteSfrE1Bit7Interrupt",
            "RouteTimer0Interrupt"
        };
        long[] mainTargets = {
            0xf80054L, 0xf8007fL, 0xf800cbL, 0xf80123L, 0xf800f6L
        };
        long[] monitorTargets = {
            0xff0304L, 0xff032fL, 0xff037bL, 0xff03d3L, 0xff03a6L
        };
        String[] monitorNames = {
            "MonitorControllerEventInterruptEntry",
            "MonitorHostInterfaceInterruptEntry",
            "MonitorControllerPacketInterruptEntry",
            "MonitorSfrE1Bit7InterruptEntry",
            "MonitorTimer0InterruptEntry"
        };
        for (int index = 0; index < routers.length; index++) {
            requireRamBytes(vectors[index], 0x02,
                (int)((routers[index] >> 8) & 0xff),
                (int)(routers[index] & 0xff));
            requireFlow(vectors[index], routers[index]);
            requireFunctionNamed(routers[index], names[index]);
            requireContiguousRamFunction(routers[index], 0x17, 8);
            requireRamBytes(routers[index],
                0xca, 0xb8, 0x7e, 0xb0, 0x00,
                0xbe, 0xb3, 0x00, 0xfc, 0x68, 0x06,
                0xda, 0xb8, 0x8a,
                (int)((mainTargets[index] >> 16) & 0xff),
                (int)((mainTargets[index] >> 8) & 0xff),
                (int)(mainTargets[index] & 0xff),
                0xda, 0xb8, 0x8a,
                (int)((monitorTargets[index] >> 16) & 0xff),
                (int)((monitorTargets[index] >> 8) & 0xff),
                (int)(monitorTargets[index] & 0xff));
            requireFlow(routers[index] + 13, mainTargets[index]);
            requireFlow(routers[index] + 19, monitorTargets[index]);
            requireFunctionNamed(monitorTargets[index], monitorNames[index]);
        }
        requireFunctionNamed(0xff4dc3L, "SelectMonitorInterruptHandlers");
        requireContiguousRamFunction(0xff4dc3L, 6, 3);
        requireRamBytes(0xff4dc3L, 0xe4, 0x7a, 0xb3, 0x00, 0xfc, 0xaa);
        requireRamInstruction(0xff4dc8L, "ERET", 1);
        println("RUNTIME_INTERRUPT_ROUTING_VALIDATION=PASS");
    }

    private void validateRuntimeBufferCommands() throws Exception {
        requireRamBytes(0xff7aec, 0x00, 0xff, 0x21, 0x0d);
        requireRamBytes(0xff7af0, 0x00, 0xff, 0x25, 0xae);
        requireFunctionNamed(0xff210d, "HandleMonitorWriteBufferCommand");
        requireFunctionNamed(0xff25ae, "HandleMonitorReadBufferCommand");
        requireRamInstruction(0xff2133, "MOV B,0x000f03", 4);
        requireRamInstruction(0xff2137, "ANL B,#0x7", 3);
        requireRamInstruction(0xff2427, "ECALL 0xff28a6", 4);
        requireRamInstruction(0xff249f, "ECALL 0xff28a6", 4);
        requireFlow(0xff2188, 0xff23cd);
        requireFlow(0xff218f, 0xff243c);
        requireFlow(0xff2427, 0xff28a6);
        requireFlow(0xff249f, 0xff28a6);
        requireRamInstruction(0xff2542, "CLR 0x0580", 2);
        requireRamInstruction(0xff2544, "CLR 0x0581", 2);
        requireRamInstruction(0xff2554, "ECALL 0xff4000", 4);
        requireRamInstruction(0xff2566, "ECALL 0xff4000", 4);
        requireFlow(0xff2554, 0xff4000);
        requireFlow(0xff2566, 0xff4000);
        println("RUNTIME_BUFFER_COMMAND_VALIDATION=PASS");
    }

    private void validateRuntimeInstructions() throws Exception {
        requireOverlayFunction("configuration_flash_relocated", 0xa8d,
            "ProgramDriveConfigurationFlashFromRam");
        requireOverlayFunction("monitor_service_relocated", 0xc0d,
            "ProgramMainFirmwareFromRam");
        requireOverlayFunction("flash_identification_relocated", 0x869,
            "DetectFlashChipFromRam");
        requireInstruction("monitor_service_relocated", 0x8c7,
            "MOV WR30,#0x0", 4);
        requireInstruction("monitor_service_relocated", 0xbf2,
            "MOV ACC,#0x99", 3);
        requireInstruction("monitor_service_relocated", 0xbf5,
            "MOV 0x0000fd,ACC", 4);
        requireInstruction("monitor_service_relocated", 0xbf9,
            "EJMP 0xff0200", 4);
        requireRamBytes(0xff021e, 0x9a, 0xff, 0x4e, 0x72);
        requireRamInstruction(0xff021e, "ECALL 0xff4e72", 4);
        requireFlow(0xff021e, 0xff4e72);
        requireInstruction("monitor_service_relocated", 0xbfd,
            "MOV ACC,R9", 2);
        requireInstruction("monitor_service_relocated", 0xc0d, "ERET", 1);

        requireRamInstruction(0xff40c9, "MOV WR18,#0x40ed", 4);
        requireRamInstruction(0xff40cd, "MOV WR16,#0xff", 4);
        requireRamInstruction(0xff40d1, "MOV DR20,#0x40e", 4);
        requireRamInstruction(0xff40d5, "MOV DR24,#0x800", 4);
        requireRamInstruction(0xff40eb, "EJMP @DR28", 2);
        requireRamInstruction(0xff45b2, "MOV WR18,#0x3c8a", 4);
        requireRamInstruction(0xff45b6, "MOV WR16,#0xff", 4);
        requireRamInstruction(0xff45ba, "MOV DR20,#0x28e", 4);
        requireRamInstruction(0xff45be, "MOV DR24,#0x800", 4);
        requireRamInstruction(0xff45d4, "EJMP @DR28", 2);
        requireRamInstruction(0xff5002, "MOV WR18,#0x5a5d", 4);
        requireRamInstruction(0xff5006, "MOV WR16,#0xff", 4);
        requireRamInstruction(0xff500a, "MOV DR20,#0x6a", 4);
        requireRamInstruction(0xff500e, "MOV WR24,#0x800", 4);
        requireRamInstruction(0xff5024, "EJMP @DR28", 2);

        validateRuntimeInterruptRouting();
        validateRuntimeIncrementDecrementFlags();
        validateRuntimeByteMultiplyDividePairs();
        require(currentProgram.getFunctionManager().getFunctionAt(
                address("RAM", 0xff0076)) != null &&
                currentProgram.getFunctionManager().getFunctionAt(
                    address("RAM", 0xff3803)) != null &&
                currentProgram.getFunctionManager().getFunctionAt(
                    address("RAM", 0xff0074)) == null &&
                currentProgram.getFunctionManager().getFunctionAt(
                    address("RAM", 0xff3802)) == null,
            "Corrected runtime boundaries are not present");
        println("RUNTIME_RELOCATION_VALIDATION=PASS");
    }

    private void validateRuntime() throws Exception {
        requireCommonIdentity(
            RUNTIME_ROLE, "data/read_memory_ff0000.debug", RUNTIME_SHA256,
            173);
        requireOption("Plextor.AssociatedFirmwareVersion", "1.04");
        requireOption("Plextor.MappedImageRange",
            "RAM:ff0000..RAM:ffffff");
        requireOption("Plextor.RelocatedOverlays",
            "configuration_flash_relocated::000800; " +
            "flash_identification_relocated::000800; " +
            "monitor_service_relocated::000800");
        require(currentProgram.getMemory().getBlocks().length == 13,
            "Expected exactly thirteen runtime memory blocks");
        requireBlock("REG_BANK_1", "RAM", 0x000000, 0x000007,
            false, false);
        requireBlock("REG_BANK_2", "RAM", 0x000008, 0x00000f,
            false, false);
        requireBlock("REG_BANK_3", "RAM", 0x000010, 0x000017,
            false, false);
        requireBlock("REG_BANK_4", "RAM", 0x000018, 0x00001f,
            false, false);
        requireBlock("INTMEM", "RAM", 0x000020, 0x0000ff,
            false, false);
        requireBlock("runtime_low_ram", "RAM", 0x000100, 0x001fff,
            false, false);
        requireBlock("RAM", "RAM", 0xff0000, 0xffffff,
            true, false);
        requireBlock("SFR", "SFR", 0x80, 0xff, false, false);
        requireBlock("BITS", "BITS", 0x100, 0x3ff, false, false);
        requireBlock("SFR-BITS", "BITS", 0x400, 0x7ff,
            false, false);
        requireOverlayCopy(
            "configuration_flash_relocated", 0xff3c8a, 0xa8d);
        requireOverlayCopy(
            "flash_identification_relocated", 0xff5a5d, 0x869);
        requireOverlayCopy(
            "monitor_service_relocated", 0xff40ed, 0xc0d);
        validateRuntimeInstructions();
        validateRuntimeBufferCommands();

        FunctionManager manager = currentProgram.getFunctionManager();
        FunctionIterator internal = manager.getFunctions(true);
        long internalCount = 0, monitor = 0, flash = 0;
        while (internal.hasNext()) {
            Function function = internal.next();
            internalCount++;
            require(tagCount(function, "unit:") == 1,
                "Runtime function lacks one unit tag at " +
                function.getEntryPoint());
            if (hasTag(function, "unit:monitor/recovery")) {
                monitor++;
            }
            if (hasTag(function, "unit:update/flash")) {
                flash++;
            }
        }
        FunctionIterator externalFunctions = manager.getExternalFunctions();
        long external = 0, externalMain = 0;
        while (externalFunctions.hasNext()) {
            Function function = externalFunctions.next();
            external++;
            require(tagCount(function, "unit:") == 1,
                "External function lacks one unit tag at " +
                function.getEntryPoint());
            if (hasTag(function, "unit:external/main_firmware")) {
                externalMain++;
            }
        }
        require(internalCount == 172 && external == 1 && monitor == 169 &&
                flash == 3 && externalMain == 1,
            "Unexpected runtime ownership metrics: internal=" +
            internalCount + " external=" + external + " monitor=" + monitor +
            " flash=" + flash + " externalMain=" + externalMain);
        println("ROLE=runtime INTERNAL_FUNCTIONS=172 EXTERNAL_FUNCTIONS=1");
        println("RUNTIME_TAGS=169_monitor+3_flash+1_external");
    }

    private void validateCanonicalProgram() throws Exception {
        requireCanonicalProjectLayout();
        String role = programInformation().getString("Plextor.Role", null);
        if (MAIN_ROLE.equals(role)) {
            validateMain();
        }
        else if (RUNTIME_ROLE.equals(role)) {
            validateRuntime();
        }
        else {
            throw new AssertionError(
                "Program is not a canonical Plextor analysis: role=" + role);
        }
        println("CANONICAL_PROJECT_VALIDATION=PASS");
    }

    @Override
    protected void run() throws Exception {
        try {
            for (String argument : getScriptArgs()) {
                if (argument.isEmpty()) {
                    continue;
                }
                if ("--prepared-baseline".equals(argument)) {
                    preparedBaseline = true;
                }
                else {
                    throw new IllegalArgumentException(
                        "Unknown argument: " + argument);
                }
            }
            validateCanonicalProgram();
        }
        catch (Throwable failure) {
            println("CANONICAL_PROJECT_VALIDATION=FAIL " +
                failure.getClass().getName() + ": " + failure.getMessage());
            for (StackTraceElement frame : failure.getStackTrace()) {
                println("  at " + frame);
            }
            if (failure instanceof Exception) {
                throw (Exception)failure;
            }
            throw new Exception(failure);
        }
    }
}
