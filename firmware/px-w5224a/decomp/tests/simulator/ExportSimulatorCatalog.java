// Export disposable simulator metadata from the canonical Ghidra program.
// @category Plextor
import java.io.BufferedWriter;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.ArrayList;
import java.util.LinkedHashSet;
import java.util.List;
import java.util.Set;

import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.AddressRange;
import ghidra.program.model.address.AddressRangeIterator;
import ghidra.program.model.listing.CodeUnit;
import ghidra.program.model.listing.Function;
import ghidra.program.model.listing.FunctionIterator;
import ghidra.program.model.listing.Instruction;
import ghidra.program.model.listing.InstructionIterator;
import ghidra.program.model.listing.Parameter;
import ghidra.program.model.symbol.Reference;

public class ExportSimulatorCatalog extends GhidraScript {
    private static String json(String value) {
        if (value == null)
            return "null";
        StringBuilder result = new StringBuilder("\"");
        for (int index = 0; index < value.length(); index++) {
            char character = value.charAt(index);
            switch (character) {
            case '\\': result.append("\\\\"); break;
            case '"': result.append("\\\""); break;
            case '\n': result.append("\\n"); break;
            case '\r': result.append("\\r"); break;
            case '\t': result.append("\\t"); break;
            default:
                if (character < 0x20)
                    result.append(String.format("\\u%04x", (int)character));
                else
                    result.append(character);
            }
        }
        return result.append('"').toString();
    }

    private static String canonical(Function function) {
        return String.format("0x%06x", function.getEntryPoint().getOffset());
    }

    private static String array(Iterable<String> values) {
        StringBuilder result = new StringBuilder("[");
        boolean first = true;
        for (String value : values) {
            if (!first)
                result.append(',');
            first = false;
            result.append(json(value));
        }
        return result.append(']').toString();
    }

    public void run() throws Exception {
        if (!currentProgram.getDomainFile().getPathname().equals("/validated/MONT_104.bin"))
            throw new Exception("Wrong program");
        if (!currentProgram.getLanguageID().toString().equals("80251-fixed:BE:24:default"))
            throw new Exception("Wrong language");
        String[] arguments = getScriptArgs();
        if (arguments.length != 1)
            throw new Exception("Expected one output path");
        Path output = Paths.get(arguments[0]);
        Files.createDirectories(output.toAbsolutePath().getParent());
        int count = 0;
        try (BufferedWriter writer = Files.newBufferedWriter(output, StandardCharsets.UTF_8)) {
            writer.write("{\"schema_version\":1,\"functions\":[");
            FunctionIterator functions = currentProgram.getFunctionManager().getFunctions(true);
            while (functions.hasNext()) {
                monitor.checkCancelled();
                Function function = functions.next();
                if (count++ != 0)
                    writer.write(',');
                List<String> ranges = new ArrayList<>();
                AddressRangeIterator rangeIterator = function.getBody().getAddressRanges();
                while (rangeIterator.hasNext()) {
                    AddressRange range = rangeIterator.next();
                    ranges.add(range.getMinAddress() + ".." + range.getMaxAddress());
                }
                Set<String> returns = new LinkedHashSet<>();
                Set<String> calls = new LinkedHashSet<>();
                Set<String> callSites = new LinkedHashSet<>();
                Set<String> memoryReferences = new LinkedHashSet<>();
                Set<String> terminalFlows = new LinkedHashSet<>();
                List<String> stockHdataInstructionAddresses = new ArrayList<>();
                InstructionIterator instructions = currentProgram.getListing().getInstructions(function.getBody(), true);
                while (instructions.hasNext()) {
                    Instruction instruction = instructions.next();
                    String mnemonic = instruction.getMnemonicString().toUpperCase();
                    byte[] instructionBytes = instruction.getBytes();
                    if (instructionBytes.length >= 3) {
                        int opcode = Byte.toUnsignedInt(instructionBytes[0]);
                        if (opcode == 0x2e || opcode == 0x4e || opcode == 0x7e)
                            stockHdataInstructionAddresses.add(
                                String.format("0x%06x", instruction.getAddress().getOffset()));
                    }
                    if (mnemonic.equals("RET") || mnemonic.equals("ERET") || mnemonic.equals("RETI"))
                        returns.add(instruction.getAddress().toString() + ":" + mnemonic.toLowerCase());
                    if (instruction.getFallThrough() == null &&
                        !mnemonic.equals("RET") && !mnemonic.equals("ERET") && !mnemonic.equals("RETI")) {
                        for (ghidra.program.model.address.Address flow : instruction.getFlows())
                            terminalFlows.add(instruction.getAddress() + ":" +
                                mnemonic.toLowerCase() + ":" + flow);
                    }
                    for (Reference reference : instruction.getReferencesFrom()) {
                        if (reference.getReferenceType().isCall()) {
                            calls.add(reference.getToAddress().toString());
                            callSites.add(instruction.getAddress() + ":" + reference.getToAddress());
                        }
                        else if (reference.isMemoryReference())
                            memoryReferences.add(reference.getToAddress().toString());
                    }
                }
                List<String> parameters = new ArrayList<>();
                for (Parameter parameter : function.getParameters())
                    parameters.add(parameter.getName() + ":" + parameter.getDataType().getName() + ":" + parameter.getVariableStorage());
                String plate = currentProgram.getListing().getComment(CodeUnit.PLATE_COMMENT, function.getEntryPoint());
                boolean abiComplete = plate == null || !plate.toLowerCase().contains("abi unknown");
                writer.write("{\"canonical_address\":" + json(canonical(function)));
                writer.write(",\"symbol\":" + json(function.getName()));
                writer.write(",\"body_ranges\":" + array(ranges));
                writer.write(",\"return_sites\":" + array(returns));
                writer.write(",\"calls\":" + array(calls));
                writer.write(",\"call_sites\":" + array(callSites));
                writer.write(",\"memory_references\":" + array(memoryReferences));
                writer.write(",\"stock_hdata_instruction_addresses\":" +
                    array(stockHdataInstructionAddresses));
                writer.write(",\"terminal_flows\":" + array(terminalFlows));
                writer.write(",\"parameters\":" + array(parameters));
                writer.write(",\"return_storage\":" + json(function.getReturn().getVariableStorage().toString()));
                writer.write(",\"custom_storage\":" + function.hasCustomVariableStorage());
                writer.write(",\"abi_complete\":" + abiComplete);
                writer.write(",\"plate_comment\":" + json(plate) + "}");
            }
            writer.write("]}\n");
        }
        if (count != 1717)
            throw new Exception("Expected 1717 functions, exported " + count);
        println("Exported " + count + " simulator function records to " + output);
    }
}
