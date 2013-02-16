package spellcheck;
import java.io.BufferedReader;
import java.io.FileReader;
import java.io.IOException;
import java.util.ArrayList;

import spellcheck.StringMatching;

public class CompareWordDoc {

	public void performCompare(String inputWord,
			                   String templatePath,
			                   String pruning) {
		// initialize a StringMatching object
		StringMatching strMatching = new StringMatching();
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

		strMatching.alignWordDoc(inputWord, 
			    			     templates,
			    			     pruning,
			    			     true, true);
	}
	
	
	/**
	 * Compare a word and a set of templates, which are saved in a file
	 */
	public static void main(String[] args) {
		// here you can specify the word and the path to the templates
        String inputWord = "Eleaphent";
        String templatePath = "/home/ymiao/Course/Speech2/assign2/dict_small.txt";
        // pruning has three options: "none", "max", "beam"
        String pruning = "none";
        
        CompareWordDoc compareWordDoc = new CompareWordDoc();
        compareWordDoc.performCompare(inputWord, templatePath, pruning);
	}

}
