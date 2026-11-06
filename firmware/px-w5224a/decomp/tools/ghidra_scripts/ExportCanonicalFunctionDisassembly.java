// Export the live canonical function inventory after confirming each body has
// decoded instructions. The output is a disposable build input, not analysis.
// @category Plextor
import java.io.BufferedWriter;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;

import ghidra.app.script.GhidraScript;
import ghidra.program.model.listing.Function;
import ghidra.program.model.listing.FunctionIterator;
import ghidra.program.model.listing.Instruction;
import ghidra.program.model.listing.InstructionIterator;

public class ExportCanonicalFunctionDisassembly extends GhidraScript {
    public void run() throws Exception {
        if (!currentProgram.getDomainFile().getPathname()
                .equals("/validated/MONT_104.bin"))
            throw new Exception("Wrong program");
        if (!currentProgram.getLanguageID().toString()
                .equals("80251-fixed:BE:24:default"))
            throw new Exception("Wrong language");

        String[] args = getScriptArgs();
        if (args.length != 1)
            throw new Exception("Expected one output path");
        Path output = Paths.get(args[0]);
        Files.createDirectories(output.toAbsolutePath().getParent());

        int count = 0;
        try (BufferedWriter writer = Files.newBufferedWriter(
                output, StandardCharsets.UTF_8)) {
            FunctionIterator functions = currentProgram.getFunctionManager()
                .getFunctions(true);
            while (functions.hasNext()) {
                monitor.checkCancelled();
                Function function = functions.next();
                InstructionIterator instructions = currentProgram.getListing()
                    .getInstructions(function.getBody(), true);
                Instruction first = null;
                Instruction last = null;
                int instructionCount = 0;
                while (instructions.hasNext()) {
                    Instruction instruction = instructions.next();
                    if (first == null)
                        first = instruction;
                    last = instruction;
                    instructionCount++;
                }
                if (first == null || currentProgram.getListing()
                        .getInstructionAt(function.getEntryPoint()) == null)
                    throw new Exception("Missing entry disassembly at " +
                        function.getEntryPoint());
                writer.write(function.getEntryPoint().toString());
                writer.write('\t');
                writer.write(function.getName());
                writer.write('\t');
                writer.write(first.getAddress().toString());
                writer.write('\t');
                writer.write(last.getMaxAddress().toString());
                writer.write('\t');
                writer.write(Integer.toString(instructionCount));
                writer.newLine();
                count++;
            }
        }
        if (count != 1717)
            throw new Exception("Expected 1717 functions, exported " + count);
        println("Exported " + count + " disassembled functions to " + output);
    }
}
