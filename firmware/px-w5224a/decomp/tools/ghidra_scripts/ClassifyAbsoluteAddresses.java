// Correlate a disposable source-address census with the canonical Ghidra map.
// This script is read-only and does not modify the program database.
// @category Plextor
import java.io.BufferedWriter;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.List;

import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.Data;
import ghidra.program.model.listing.Function;
import ghidra.program.model.listing.Instruction;
import ghidra.program.model.mem.MemoryBlock;
import ghidra.program.model.symbol.Symbol;

public class ClassifyAbsoluteAddresses extends GhidraScript {
    private String clean(String value) {
        if (value == null)
            return "";
        return value.replace('\t', ' ').replace('\n', ' ');
    }

    public void run() throws Exception {
        if (!currentProgram.getDomainFile().getPathname()
                .equals("/validated/MONT_104.bin"))
            throw new Exception("Wrong program");
        if (!currentProgram.getLanguageID().toString()
                .equals("80251-fixed:BE:24:default"))
            throw new Exception("Wrong language");

        String[] args = getScriptArgs();
        if (args.length != 2)
            throw new Exception("Expected input address list and output TSV");
        Path input = Paths.get(args[0]);
        Path output = Paths.get(args[1]);
        Files.createDirectories(output.toAbsolutePath().getParent());
        List<String> lines = Files.readAllLines(input, StandardCharsets.US_ASCII);

        int count = 0;
        try (BufferedWriter writer = Files.newBufferedWriter(
                output, StandardCharsets.UTF_8)) {
            writer.write("address\tclassification\towner_start\towner_end\towner_name\ttype\tmemory_block");
            writer.newLine();
            for (String text : lines) {
                monitor.checkCancelled();
                if (text.trim().isEmpty())
                    continue;
                Address address = currentProgram.getAddressFactory()
                    .getAddress(text.trim());
                if (address == null)
                    throw new Exception("Invalid address: " + text);

                String classification = "undefined";
                String start = "";
                String end = "";
                String name = "";
                String type = "";
                Function function = currentProgram.getFunctionManager()
                    .getFunctionContaining(address);
                Data data = currentProgram.getListing()
                    .getDefinedDataContaining(address);
                Instruction instruction = currentProgram.getListing()
                    .getInstructionAt(address);
                Symbol symbol = currentProgram.getSymbolTable()
                    .getPrimarySymbol(address);
                if (function != null) {
                    classification = address.equals(function.getEntryPoint())
                        ? "function_entry" : "function_interior";
                    start = function.getEntryPoint().toString();
                    end = function.getBody().getMaxAddress().toString();
                    name = function.getName();
                    type = function.getSignature().toString();
                } else if (data != null) {
                    classification = "data";
                    start = data.getMinAddress().toString();
                    end = data.getMaxAddress().toString();
                    name = data.getLabel();
                    type = data.getDataType().getDisplayName();
                } else if (instruction != null) {
                    classification = "instruction_without_function";
                    start = instruction.getMinAddress().toString();
                    end = instruction.getMaxAddress().toString();
                    name = symbol == null ? "" : symbol.getName();
                    type = instruction.getMnemonicString();
                } else if (symbol != null) {
                    classification = "symbol";
                    start = address.toString();
                    end = address.toString();
                    name = symbol.getName();
                }
                MemoryBlock block = currentProgram.getMemory().getBlock(address);
                writer.write(address.toString());
                writer.write('\t');
                writer.write(classification);
                writer.write('\t');
                writer.write(start);
                writer.write('\t');
                writer.write(end);
                writer.write('\t');
                writer.write(clean(name));
                writer.write('\t');
                writer.write(clean(type));
                writer.write('\t');
                writer.write(block == null ? "" : clean(block.getName()));
                writer.newLine();
                count++;
            }
        }
        println("Classified " + count + " source addresses into " + output);
    }
}
