package spellcheck;

import java.io.BufferedReader;
import java.io.FileReader;
import java.io.IOException;
import java.util.ArrayList;

import spellcheck.Utils;
/**
 * This program computes the word alignment and word error rate (WER),
 * based on Dynamic Time Warping. The reference and hypothesis texts 
 * are in two separate files. This program takes these two file names
 * as input arguments.
 *
 */
public class WordErrorRate {

	private int errorTokenLen = 3;    //the number of error types, which is 3 in this case
	
	/**
	 * get word alignment and WER for all the reference-hypothesis pairs
	 * the first thing is to read these pairs from the input files
	 */
	public void computeWER(String hyposPath, String referPath) {
		
		// initialize the Utils object
		Utils utils = new Utils();
		
		//read all the reference sentences
		ArrayList<String> referStrings = new ArrayList<String>();
		try {
			String oneLine = "";
			BufferedReader bfReader = new BufferedReader(new FileReader(referPath));
			while ((oneLine = bfReader.readLine()) != null) {
				oneLine = utils.processWord(oneLine);
				if (oneLine == "" || oneLine.equals("")) continue;
				referStrings.add(oneLine);
			}
			bfReader.close();
		} catch(IOException e) {
			e.printStackTrace();
		}
		
		//read all the hypothesis sentences
		ArrayList<String> hyposStrings = new ArrayList<String>();
		try {
			String oneLine = "";
			BufferedReader bfReader = new BufferedReader(new FileReader(hyposPath));
			while ((oneLine = bfReader.readLine()) != null) {
				if (oneLine == "" || oneLine.equals("")) continue;
				hyposStrings.add(oneLine);
			}
			bfReader.close();
		} catch(IOException e) {
			e.printStackTrace();
		}
		
		//there will be an error, if the number of hypothesis and reference sentences are not the same
		if (referStrings.size() != hyposStrings.size()) {
			System.err.println("Error: the hypos and ref files should have the same number of sentences");
			return;
		}
		
		int[] total_errors = new int[errorTokenLen];
		for (int e = 0; e < errorTokenLen; e++)
			total_errors[e] = 0;
		
		for (int i = 0; i < referStrings.size(); i++) {
			int[] sentence_errors = getWordAlignment(utils.processWord(hyposStrings.get(i)).trim(),
					                                 utils.processWord(referStrings.get(i).trim()),
					                                 utils);
			
			for (int e = 0; e < errorTokenLen; e++)
				total_errors[e] += sentence_errors[e];
		}
		
		System.out.println("ERRORS " + (total_errors[0] + total_errors[1] + total_errors[2])
				+ " [ INS " + total_errors[0] + ", DEL " + total_errors[1]
				+ ", SUB " + total_errors[2] + " ]");
	}
	
	/**
	 * get the word alignment for each pair of sentences
	 * @param hyposString
	 * @param refreString
	 * @return the DTW distance between the two sentences
	 */
	private int[] getWordAlignment(String inputString, String templateString, Utils utils) {
		
		// array to record the number of errors: INS DEL SUB
		int[] alignErrors = new int[errorTokenLen];
		for (int i = 0; i < errorTokenLen; i++)
			alignErrors[i] = 0;
		
		// critical: add the dumpy "dumpy"
		String[] inputArr = ("dumpy " + inputString).split(" ");
		String[] templateArr = ("dumpy " + templateString).split(" ");
		
		int n = inputArr.length; int m = templateArr.length;
		// DTW matrix
		double[][] levenDist = new double[m][n];
		levenDist[0][0] = 0;
		//traceI or traceJ is a matrix which records the backpointers
		int[][] traceI = new int[m][n];
		int[][] traceJ = new int[m][n];

		// i (index for template)   j (index for input)
		int i, j, tempI, tempJ; double insertCost, subsCost;
		
		for (i = 1; i < m; i++) { levenDist[i][0] = i; traceI[i][0] = i - 1; traceJ[i][0] = 0; }
		for (j = 1; j < n; j++) { levenDist[0][j] = j; traceI[0][j] = 0; traceJ[0][j] = j - 1; }
		
		//compute the DTW matrix
		for (j = 1; j < n; j++) {
			for (i = 1; i < m; i++) {
				// deletion
				levenDist[i][j] = levenDist[i - 1][j] + 1; traceI[i][j] = i - 1; traceJ[i][j] = j;
				// substitution
				subsCost = levenDist[i - 1][j - 1];
				// comparing two strings, rather than letters
				if (!inputArr[j].equals(templateArr[i]) && inputArr[j] != templateArr[i]) {
					subsCost++;
				}
				if (levenDist[i][j] > subsCost) {
					levenDist[i][j] = subsCost;
					traceI[i][j] = i - 1; traceJ[i][j] = j - 1;
				}
				// insertion
				if (levenDist[i][j - 1] >= 0) {
					insertCost = levenDist[i][j - 1] + 1;
					if (levenDist[i][j] > insertCost) {
						levenDist[i][j] = insertCost;
						traceI[i][j] = i; traceJ[i][j] = j - 1;
					}
				}
			} // end of i
		} // end of j
		
		//determine and record word error type (INS, DEL or SUB) at each hypothesis point
		//Note: when the hypothesis and reference matches, we still record it as SUB
		i = m-1; j = n-1;
		ArrayList<String> errorType = new ArrayList<String>();
		while (i != 0 || j != 0) {
			tempI = traceI[i][j];
			tempJ = traceJ[i][j];
			
			if (tempI == i && tempJ == (j - 1)) {              //insertion
				errorType.add("INS"); alignErrors[0]++;
			} else if (tempI == (i - 1) && tempJ == j) {       //deletion
				errorType.add("DEL"); alignErrors[1]++;
			} else if (tempI == (i - 1) && tempJ == (j - 1)) { //substitution
				errorType.add("SUB");
				if (!inputArr[j].equals(templateArr[i])) {
					alignErrors[2]++;
				}
			}
			i = tempI; j = tempJ;
		}
		// we assume that the first word is "dumpy"
		errorType.add("SUB");
		
		//display the word alignment
		utils.displayWordAlignment(errorTokenLen, errorType, inputArr, templateArr, alignErrors);
		
		return alignErrors;
	}
	
	public static void main(String args[]) {

		// a toy example to demonstrate whether the codes are correct
//      String inputPath = "/home/ymiao/Course/Speech2/assign2/asr_hypos";
//      String templatePath = "/home/ymiao/Course/Speech2/assign2/asr_refer";
	
        // templatePath points to the set of templates (dictionary)
		// inputPath points to the plain text which need to be spellchecked
        String inputPath = "/home/ymiao/Course/Speech2/assign2/story_checked.txt";
//		String inputPath = "/home/ymiao/Course/Speech2/assign2/story.txt";
        String templatePath = "/home/ymiao/Course/Speech2/assign2/storycorrect.txt";
		
		WordErrorRate wordErrorRate = new WordErrorRate();
		wordErrorRate.computeWER(inputPath, templatePath);
	}
}
