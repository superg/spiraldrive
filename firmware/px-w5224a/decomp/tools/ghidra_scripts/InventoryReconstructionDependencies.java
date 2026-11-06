// Read-only disposable dependency inventory for bottom-up source reconstruction.
// @category Plextor
import ghidra.app.script.GhidraScript;
import ghidra.program.model.listing.*;

public class InventoryReconstructionDependencies extends GhidraScript {
    public void run() throws Exception {
        if (!currentProgram.getDomainFile().getPathname().equals("/validated/MONT_104.bin"))
            throw new Exception("Wrong program");
        if (!currentProgram.getLanguageID().toString().equals("80251-fixed:BE:24:default"))
            throw new Exception("Wrong language");
        FunctionIterator functions = currentProgram.getFunctionManager().getFunctions(true);
        while (functions.hasNext()) {
            monitor.checkCancelled();
            Function function = functions.next();
            StringBuilder line = new StringBuilder(function.getEntryPoint().toString())
                .append("\t").append(function.getName())
                .append("\t").append(function.getBody().getNumAddresses()).append("\t");
            for (Function callee : function.getCalledFunctions(monitor))
                line.append(callee.getEntryPoint().toString()).append(",");
            println(line.toString());
        }
    }
}
