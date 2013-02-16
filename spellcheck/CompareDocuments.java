package spellcheck;
import java.io.BufferedReader;
import java.io.FileReader;
import java.io.IOException;
import java.io.File;
import java.util.ArrayList;

import spellcheck.StringMatching;
import spellcheck.Utils;

public class CompareDocuments {

	public void performCompare(String inputPath,
			                   String templatePath,
			                   String outputPath,
			                   String pruning) {
		// initialize a StringMatching object
		StringMatching strMatching = new StringMatching();
		Utils utils = new Utils();
		// check whether the output file has existed; if yes, delete it
		File file = new File(outputPath);
		if (file.exists()) file.delete();
		
		// read all the templates
		ArrayList<String> templates = new ArrayList<String>();
		try {
			String oneLine = "";
			BufferedReader bfReader = new BufferedReader(new FileReader(templatePath));
			while ((oneLine = bfReader.readLine()) != null) {
				templates.add(oneLine);
			}
			bfReader.close();
		} catch(IOException e) {
			e.printStackTrace();
		}
		
		// now match the input doc word-by-word
		try {
			String oneLine = "";
			BufferedReader bfReader = new BufferedReader(new FileReader(inputPath));
			while ((oneLine = bfReader.readLine()) != null) {
				if (oneLine.equals("") || oneLine == "") {
					System.out.println();
					utils.outOneline(outputPath, "");
					continue;
				}
			    String[] inputWords = (utils.processWord(oneLine)).split(" ");
			    oneLine = "";
			    for (int i = 0; i < inputWords.length; i++) {
			    	int matchIndex = strMatching.alignWordDoc(inputWords[i], 
			    			                                  templates,
			    			                                  pruning,
			    			                                  false, false);
			    	if (matchIndex == -1) {
			    		oneLine = oneLine + "****" + " ";
			    	} else {
			    		oneLine = oneLine + templates.get(matchIndex) + " ";
			    	}
			    }
			    System.out.println(oneLine);
			    utils.outOneline(outputPath, oneLine.trim());
			}
			bfReader.close();
		} catch(IOException e) {
			e.printStackTrace();
		}
	}
	
	
	/**
	 * Compare two documents
	 */
	public static void main(String[] args) {
		// templatePath points to the set of templates (dictionary)
		// inputPath points to the plain text which need to be spellchecked
        String inputPath = "/home/ymiao/Course/Speech2/assign2/story.txt";
        String templatePath = "/home/ymiao/Course/Speech2/assign2/dict.txt";
        // pruning has three options: "none", "max", "beam"
        String pruning = "none";
        // outputPath points to the file where the results are saved
        String outputPath = "/home/ymiao/Course/Speech2/assign2/story_checked.txt";
        
        CompareDocuments compareDocs = new CompareDocuments();
        compareDocs.performCompare(inputPath, templatePath, outputPath, pruning);
	}

}
