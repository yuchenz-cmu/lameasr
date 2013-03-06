package isolaterec;
import java.io.BufferedReader;
import java.io.FileReader;
import java.io.IOException;
import java.io.File;
import java.util.ArrayList;

import isolaterec.UtteranceMatching;

public class Recognition {

	public void performRec(String inputPath,
			               String templatePath,
			               boolean doDelta,
			               String pruning,
			               double prune_beam) {
		
		// read all the templates
		ArrayList<String> templateTexts = new ArrayList<String>();
		ArrayList<ArrayList<String>> templateUtts = new ArrayList<ArrayList<String>>();
		try {
			String oneLine = "";
			BufferedReader bfReader = new BufferedReader(new FileReader(templatePath));
			while ((oneLine = bfReader.readLine()) != null) {
				String[] str = oneLine.split(" ");
				templateTexts.add(str[0]);
				ArrayList<String> templateUtt = this.readOneUtt(str[1]);
				templateUtts.add(templateUtt);
			}
			bfReader.close();
		} catch(IOException e) {
			e.printStackTrace();
		}
		
		UtteranceMatching uttMatching = new UtteranceMatching(templateUtts,
				                                             templateTexts,
				                                             doDelta,
				                                             prune_beam);
		
		// now match the input doc word-by-word
		int input_num = 0; double correct_num = 0;
		try {
			String oneLine = "";
			BufferedReader bfReader = new BufferedReader(new FileReader(inputPath));
			while ((oneLine = bfReader.readLine()) != null) {
				
			    String[] str = oneLine.split(" ");
			    ArrayList<String> inputUtt = this.readOneUtt(str[1]);
			    
			    String recResult = uttMatching.alignUttTemplates(inputUtt,
	                                                             str[0],
	                                                             pruning, 
	                                                             true);
			    
			    if (recResult.equals(str[0]) || recResult == str[0]) {
			    	correct_num ++;
			    }
			    input_num++;
			}
			bfReader.close();
		} catch(IOException e) {
			e.printStackTrace();
		}
		
		System.out.println("Recognition Accuracy: " + (correct_num / input_num));

	}
	
	private ArrayList<String> readOneUtt(String path) {
		ArrayList<String> utt = new ArrayList<String>();
		// now match the input doc word-by-word
		try {
			String oneLine = "";
			BufferedReader bfReader = new BufferedReader(new FileReader(path));
			while ((oneLine = bfReader.readLine()) != null) {
				utt.add(oneLine);
			}
			bfReader.close();
		} catch(IOException e) {
			e.printStackTrace();
		}
		return utt;
	}
	
	/**
	 * Compare two documents
	 */
	public static void main(String[] args) {
		// templatePath points to the set of templates (dictionary)
		// inputPath points to the plain text which need to be spellchecked
		String trainPath = "list_mix/5_train.list";
        String testPath = "list_mix/5_test.list";
        // pruning has three options: "none", "beam"

		String pruning = "beam";
		double beam = 55;
        boolean doDelta = true;
        
        
        Recognition recognition = new Recognition();
        recognition.performRec(testPath, trainPath, doDelta, pruning, beam);

        // list_mixture: 2_train 5_test  0.86
        // list_cross: 2_train 5_test    0.62
        
	}

}
