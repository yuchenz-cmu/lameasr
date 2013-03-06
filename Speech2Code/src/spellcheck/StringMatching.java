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
 *You can type "WordErrorRate" in the command line to get help info.
 * 
 * @author Yajie Miao
 *
 */
public class StringMatching {
	
	private double max_prune_thresh = 3.0;    // a maximum string edit distance of 3
	private double beam_prune_thresh = 3.0;   // a "beam" of 3 relative to the current best score
	
	/**
	 * string matching for two words
	 * @param input
	 * @param template
	 * @param pruning
	 * @return NULL
	 */
	public void alignTwoWords(String input, String template, String pruning) {
		
		// critical: add the dumpy "d"
		char[] inputArr = ("d" + input).toCharArray();
		char[] templateArr = ("d" + template).toCharArray();
		
		int n = inputArr.length; int m = templateArr.length;
		// DTW matrix
		double[][] levenDist = new double[m][n];
		levenDist[0][0] = 0;
		//traceI or traceJ is a matrix which records the backpointers
		int[][] traceI = new int[m][n];
		int[][] traceJ = new int[m][n];

		// i (index for template)   j (index for input)
		int i, j; double insertCost, subsCost;
		
		for (i = 1; i < m; i++) { levenDist[i][0] = i; traceI[i][0] = i - 1; traceJ[i][0] = 0; }
		for (j = 1; j < n; j++) { levenDist[0][j] = j; traceI[0][j] = 0; traceJ[0][j] = j - 1; }
		
		//compute the DTW matrix
		for (j = 1; j < n; j++) {
			// record the minimum distance at the current column
			double min_value = Double.MAX_VALUE;
			for (i = 1; i < m; i++) {
				// deletion
				levenDist[i][j] = levenDist[i - 1][j] + 1; traceI[i][j] = i - 1; traceJ[i][j] = j;
				// substitution
				if (levenDist[i - 1][j - 1] >= 0) {
					subsCost = levenDist[i - 1][j - 1] + ((inputArr[j] == templateArr[i])? 0:1);
					if (levenDist[i][j] > subsCost) {
						levenDist[i][j] = subsCost;
						traceI[i][j] = i - 1; traceJ[i][j] = j - 1;
					}
				}
				// insertion
				if (levenDist[i][j - 1] >= 0) {
					insertCost = levenDist[i][j - 1] + 1;
					if (levenDist[i][j] > insertCost) {
						levenDist[i][j] = insertCost;
						traceI[i][j] = i; traceJ[i][j] = j - 1;
					}
				}
				if (levenDist[i][j] >= 0 && levenDist[i][j] < min_value) {
					min_value = levenDist[i][j];
				}
			} // end of i
			
			// beam pruning
			int num_above_zero = 0;
			if (pruning.equals("none")) continue;
			for (i = 1; i < m; i++) {
				if (pruning.equals("max") && levenDist[i][j] > max_prune_thresh) {
					levenDist[i][j] = -1;
					continue;
				}
				if (pruning.equals("beam") && levenDist[i][j] > (min_value + beam_prune_thresh)) {
					levenDist[i][j] = -1;
					continue;
				}
				num_above_zero++;
			}
			if (num_above_zero == 0) {
				System.err.println("Word [" + input + "] cannot be matched"
						+ "against [" + template + "], due to pruning");
				return;
			}
		}
		
		System.out.println("Distance of [" + input + "] against [" + template
				+ "] = " + levenDist[m-1][n-1]);
		// Show the trellis
		ArrayList<String> best_path = new ArrayList<String>();
		int temp_i = -1;
		i = m - 1; j = n - 1;
		best_path.add(i + " " + j);
		while (i != 0 || j != 0) {
			temp_i = traceI[i][j]; j = traceJ[i][j];
			i = temp_i;
			best_path.add(i + " " + j);
		}
		Utils utils = new Utils();
		utils.displayTrellis(levenDist, best_path, inputArr, templateArr);
	}
	
	public int alignWordDoc(String inputWord,
			                 ArrayList<String> templateStrs,
			                 String pruning,
			                 boolean showTrellis, 
			                 boolean showResult) {
		
		int templateSize = templateStrs.size();          // # of templates
		int[] templateLengths = new int[templateSize];   // length of each template (including "d")
		int[] templateIndexes = new int[templateSize];
		String template_concat = "";
		for (int t = 0; t < templateSize; t++) {
			template_concat = template_concat + "d" + templateStrs.get(t);
			templateLengths[t] = (templateStrs.get(t).toCharArray()).length + 1;
			if (t == 0) templateIndexes[t] = 0;
			else templateIndexes[t] = templateIndexes[t-1] + templateLengths[t-1];
		}
		
		char[] templateArr = template_concat.toCharArray();
		int m = templateArr.length;
		
		char[] inputArr = ("d" + inputWord).toCharArray();
	    int n = inputArr.length;
	    //DTW matrix
		double[][] levenDist = new double[m][n];
		//traceI or traceJ is a matrix which records the tracing-back indexes
		int[][] traceI = new int[m][n];
		int[][] traceJ = new int[m][n];
		
	    int t, i, j; double insertCost, subsCost, deleteCost;
        // initialize the DTW matrix and backpointers
		for (t = 0; t < templateSize; t++) {
			levenDist[templateIndexes[t]][0] = 0;
			for (j = 1; j < n; j++) {
				levenDist[templateIndexes[t]][j] = j;
				traceI[templateIndexes[t]][j] = templateIndexes[t];
				traceJ[templateIndexes[t]][j] = j - 1;
			}
			for (i = 1; i < templateLengths[t]; i++) { 
				levenDist[templateIndexes[t] + i][0] = i;
				traceI[templateIndexes[t] + i][0] = templateIndexes[t] + i - 1;
				traceJ[templateIndexes[t] + i][0] = 0;
			}
		}
			
		// indicator about whether the current template is active   1: active   0: inactive
		int[] templateStates = new int[templateSize];
		for (t = 0; t < templateSize; t++) templateStates[t] = 1;
		//compute the DTW matrix
		for (j = 1; j < n; j++) {
		  double min_value = Double.MAX_VALUE;
		  for (t = 0; t < templateSize; t++) {
			  if (templateStates[t] == 0) continue;
			    for (i = (templateIndexes[t] + 1); i < (templateIndexes[t] + templateLengths[t]); i++) {
	                // deletion
			    	levenDist[i][j] = levenDist[i - 1][j] + 1; traceI[i][j] = i - 1; traceJ[i][j] = j;
					// substitution
					if (levenDist[i - 1][j - 1] >= 0) {
					    subsCost = levenDist[i - 1][j - 1] + ((inputArr[j] == templateArr[i])? 0:1);
						if (levenDist[i][j] > subsCost) {
						  levenDist[i][j] = subsCost;
						  traceI[i][j] = i - 1; traceJ[i][j] = j - 1;
						}
					}
					// insertion
					if (levenDist[i][j - 1] >= 0) {
					  insertCost = levenDist[i][j - 1] + 1;
					  if (levenDist[i][j] > insertCost) {
					    levenDist[i][j] = insertCost;
						traceI[i][j] = i; traceJ[i][j] = j - 1;
					  }
					}
					if (levenDist[i][j] >= 0 && levenDist[i][j] < min_value) {
					  min_value = levenDist[i][j];
					}
				} // end of i
			} // end of t
				    
			// beam pruning
		    // we don't do pruning on the last column
			if (pruning.equals("none") || j == (n - 1)) continue;
				
		    int total_num_above_zero = 0;
			for (t = 0; t < templateSize; t++) {
				if (templateStates[t] == 0) continue;
			  	int num_above_zero = 0;
				for (i = (templateIndexes[t] + 1); i < (templateIndexes[t] + templateLengths[t]); i++) {
				  	if (pruning.equals("max") && levenDist[i][j] > 3) {
				        levenDist[i][j] = -1;
						continue;
					}
					if (pruning.equals("beam") && levenDist[i][j] > (min_value + 3)) {
						levenDist[i][j] = -1;
						continue;
					}
					num_above_zero++;
				}
				if (num_above_zero == 0) {
				    templateStates[t] = 0;
				}
				total_num_above_zero += num_above_zero;
			} // end of t
			if (total_num_above_zero == 0) {   // all the templates have died
				System.err.println("Word [" + inputWord + "] cannot be matched"
						+ "against the templates, due to pruning");
				return -1;
			}
				
		} // end of j
			
		// Now we need to find the best score
		int bestTemplateIndex = -1; double bestDtwScore = Double.MAX_VALUE;
		for (t = 0; t < templateSize; t++) {
			if (templateStates[t] == 0) continue;
			double thisDtwScore = levenDist[templateIndexes[t] + templateLengths[t] - 1][n - 1];				
			if (bestDtwScore > thisDtwScore && thisDtwScore >= 0) {
				bestDtwScore = thisDtwScore;
				bestTemplateIndex = t;
			}
		}
		
		if (bestTemplateIndex == -1) {
			System.err.println("Word [" + inputWord + "] cannot be matched"
					+ "against the templates, likely due to pruning");
			return -1;
		}
		
		if (showResult) {
		    System.out.println("Best match of [" + inputWord + "] is ["
				+ templateStrs.get(bestTemplateIndex) + "] with distance = "
				+ levenDist[templateIndexes[bestTemplateIndex] + templateLengths[bestTemplateIndex] - 1][n - 1]);
		}
		
		// show trellis
		if (showTrellis) {
			// Show the trellis
			ArrayList<String> best_path = new ArrayList<String>();
			int temp_i = -1;
			i = templateIndexes[bestTemplateIndex] + templateLengths[bestTemplateIndex] - 1;
			j = n - 1;
			best_path.add(i + " " + j);
			while (i != templateIndexes[bestTemplateIndex] || j != 0) {
				temp_i = traceI[i][j]; j = traceJ[i][j];
				i = temp_i;
				best_path.add(i + " " + j);
			}
			Utils utils = new Utils();
			utils.displayTrellisDoc(levenDist, best_path, inputArr, templateArr, templateIndexes);
		}
		
		
		return bestTemplateIndex;
	}
	
	public static void main(String args[]) {

//		if (args.length != 2) {
//			System.out.println("Error: WordErrorRate must take two arguments");
//			System.out.println("WordErrorRate -refFile -hypFile");
//			System.out.println("-refFile: the pointer to the reference file");
//			System.out.println("-hypFile: the pointer to the hypothesis file");
//			return;
//		}
//		
//		WordErrorRate wordErrorRate = new WordErrorRate(args[0], args[1]);
//		wordErrorRate.computeWER();
		
//		String str1 = "Eleaphent"; 
//		char[] charArr1 = str1.toCharArray();
//		String str2 = "ApplePie"; 
//		char[] charArr2 = str2.toCharArray();
//		WordErrorRate wer = new WordErrorRate();
//		System.out.print(wer.getWordAlignment(str1, str2, "beam"));
		
		StringMatching strMatch = new StringMatching();
		ArrayList<String> templates = new ArrayList<String>();
		templates.add("once"); templates.add("upon"); templates.add("time");
		templates.add("while"); templates.add("king");
		
		strMatch.alignWordDoc("whise", templates, "max", true, true);
		
//		
//		ArrayList<String> inputs = new ArrayList<String>();
//		inputs.add("onse"); inputs.add("apon"); inputs.add("tyme"); inputs.add("wile");
//		inputs.add("kng");
		
//		StringMatching wer = new StringMatching();
//		wer.alignTwoDocs(inputs, templates, "max");
//		System.out.println(wer.alignTwoWords("wile", "while", "beam"));
		
//		StringMatching strMatch = new StringMatching();
//		strMatch.alignTwoWords("Elegant", "Elephant", "none");
		
	}
}
