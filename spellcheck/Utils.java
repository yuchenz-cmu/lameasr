package spellcheck;
import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.util.ArrayList;

/**
 * This program computes the word alignment and word error rate (WER),
 * based on Dynamic Time Warping. The reference and hypothesis texts 
 * are in two separate files. This program takes these two file names
 * as input arguments.
 *
 *You can type "WordErrorRate" in the command line to get help info.
 * 
 * @author Yajie Miao
 *
 */
public class Utils {
	
	public void displayTrellis(double[][] trellis,
			                   ArrayList<String> best_path,
			                   char[] inputArr,
			                   char[] templateArr) {
		int m = trellis.length;
		int n = trellis[0].length;
		for (int i = (m - 1); i >= 0; i--) {
			System.out.print(templateArr[i] + "  |  ");
			for (int j = 0; j < n; j++) {
				String suffix = "";
				if (best_path.contains(i + " " + j)) {
					suffix = "*";
				}
			    if (j < (n - 1)) System.out.print(trellis[i][j] + suffix + "\t");
			    else System.out.print(trellis[i][j] + suffix + "\n");
			}
		}
		
		System.out.print("      ");
		for (int j = 0; j < n; j++) {
			if (j < (n - 1)) System.out.print("---\t");
			else System.out.print("---\n");
		}
		System.out.print("      ");
		for (int j = 0; j < n; j++) {
			if (j < (n - 1)) System.out.print(" " + inputArr[j] + " " + "\t");
			else System.out.print(" " + inputArr[j] + " " + "\n");
		}
	}
	
	
	public void displayTrellisDoc(double[][] trellis,
			                      ArrayList<String> best_path,
			                      char[] inputArr,
			                      char[] templateArr,
			                      int[] templateIndexes) {
		int m = trellis.length;
		int n = trellis[0].length;
		ArrayList<Integer> indexArr = new ArrayList<Integer>();
		for (int i = 0; i < templateIndexes.length; i++) {
			indexArr.add(templateIndexes[i]);
		}
		
		for (int i = (m - 1); i >= 0; i--) {
			System.out.print(templateArr[i] + "  |  ");
			for (int j = 0; j < n; j++) {
				String suffix = "";
				if (best_path.contains(i + " " + j)) {
					suffix = "*";
				}
			    if (j < (n - 1)) System.out.print(trellis[i][j] + suffix + "\t");
			    else System.out.print(trellis[i][j] + suffix + "\n");
			}
			if (indexArr.contains(i) && i > 0)
				System.out.println();
		}
		
		System.out.print("      ");
		for (int j = 0; j < n; j++) {
			if (j < (n - 1)) System.out.print("---\t");
			else System.out.print("---\n");
		}
		System.out.print("      ");
		for (int j = 0; j < n; j++) {
			if (j < (n - 1)) System.out.print(" " + inputArr[j] + " " + "\t");
			else System.out.print(" " + inputArr[j] + " " + "\n");
		}
	}
	
	public void outOneline (String outputPath, String oneLine) {
		try {
			BufferedWriter bfWriter = new BufferedWriter(new FileWriter(outputPath, true));
			bfWriter.write(oneLine + "\n");
			bfWriter.close();
		} catch(IOException e) {
			e.printStackTrace();
		}
	}		

	
	public String processWord(String word) {
		return word.replace(",", "").replace(".", "").replace("\"", "")
		    .replace("!", "").replace(";", "").replace("?", "")
		    .replace(":", "").toLowerCase();
	}
	
	public void displayWordAlignment(int errorTokenLen,
			                         ArrayList<String> errorType,
			                         String[] inputArr,
			                         String[] templateArr,
			                         int[] alignErrors) {
		//generate the final output, based on the error types
		int i = 0, j = 0;
		int m = templateArr.length;
		int n = inputArr.length;
		String refOutput = "", hypOutput = "";
		for (int e = (errorType.size() - 1); e >= 0; e--) {
			
			String hyposWord = inputArr[j];    //the word at point i in the hypothesis
			String refreWord = templateArr[i];    //the word at point j in the reference
			int hyposLength = hyposWord.length(); //the length of this hypothesis word
			int refreLength = refreWord.length(); //the length of this reference word
			
			String error = errorType.get(e);
			
			if (error.equals("SUB") || error == "SUB") {     //substitution
				
				hypOutput = hypOutput + hyposWord + " ";
				refOutput = refOutput + refreWord + " ";
				
				//compare the length of the hypothesis and reference words
				//organize the output by adding " "
				if (hyposLength > refreLength) {
					refOutput = addSpaceAfter(refOutput, hyposLength - refreLength);
				} else {
					hypOutput = addSpaceAfter(hypOutput, refreLength - hyposLength);
				}
				
				int maxLength = hyposLength;
				if (maxLength < refreLength) maxLength = refreLength;
				
				if (i < m) i++; 
				if (j < n) j++;
				
			} else if (error.equals("DEL") || error == "DEL") {    //deletion
				
				refOutput = refOutput + refreWord + " ";
				hypOutput = hypOutput + " ";
				hypOutput = addSpaceAfter(hypOutput, refreLength);
				
				if (i < m) i++;
			
			} else {       //insertion
				
				hypOutput = hypOutput + hyposWord + " ";
				refOutput = refOutput + " ";
				refOutput = addSpaceAfter(refOutput, hyposLength);
				
				if (j < n) j++;
			}
		}
		
		System.out.println("REF: " + refOutput);
		System.out.println("HYP: " + hypOutput + "\t[ INS " + alignErrors[0]
		             + ", DEL " + alignErrors[1] + ", SUB " + alignErrors[2] + " ]");
		System.out.println();
	}
	
	/**
	 * add #number " " at the end of oriString
	 * @param oriString
	 * @param number
	 * @return the string with " " added
	 */
	private String addSpaceAfter(String oriString, int number) {
		for (int i = 0; i < number; i++) {
			oriString += " ";
		}
		return oriString;
	}
	
}
