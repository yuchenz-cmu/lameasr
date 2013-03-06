package isolaterec;
import java.io.BufferedReader;
import java.io.FileReader;
import java.io.IOException;
import java.util.ArrayList;

import spellcheck.Utils;

/**
 * 
 * @author Yajie Miao
 *
 */
public class UtteranceMatching {
	
	private double beam_prune_thresh = 20.0;   // a "beam" of 3 relative to the current best score
	
	private int delta_window = 2;
	private int raw_mfcc_dim = 13;
	
	private int templateNum;
	private int templateVecSize;
	private ArrayList<String> templateTexts;
	private int[] templateLengths;
	private int[] templateIndexes;
	private double[][] template_mat;
	
	private boolean doDelta;
	
	public UtteranceMatching (ArrayList<ArrayList<String>> templateUtts,
		                       ArrayList<String> templateTexts,
			                   boolean doDelta,
			                   double prune_beam) {
		
		this.beam_prune_thresh = prune_beam;
		this.doDelta = doDelta;
		this.templateTexts = templateTexts;
		
		templateNum = templateUtts.size();          // # of templates
		templateVecSize = 0;                        // # of feature vectors in templates                        
		templateLengths = new int[templateNum];   // length of each template (including "d")
		templateIndexes = new int[templateNum];
		for (int t = 0; t < templateNum; t++) {
			templateLengths[t] = templateUtts.get(t).size();
			templateVecSize += templateLengths[t];
			if (t == 0) templateIndexes[t] = 0;
			else templateIndexes[t] = templateIndexes[t-1] + templateLengths[t-1];
		}
		
		// make the feat (mfcc+delta+double_delta) matrix for all the templates
		if (doDelta) {
            template_mat = new double[templateVecSize][3 * raw_mfcc_dim];
		} else {
			template_mat = new double[templateVecSize][raw_mfcc_dim];
		}
		
		for (int t = 0; t < templateNum; t++) {
			double[][] feat_mat = convertUtt2Mat(templateUtts.get(t), doDelta);
			for (int m = templateIndexes[t]; m < templateIndexes[t] + templateLengths[t]; m++) {
				if (doDelta) {
				    for (int d = 0; d < 3 * raw_mfcc_dim; d++) {
					    template_mat[m][d] = feat_mat[m - templateIndexes[t]][d];
				    }
				} else {
					for (int d = 0; d < raw_mfcc_dim; d++) {
					    template_mat[m][d] = feat_mat[m - templateIndexes[t]][d];
				    }
				}
			}
		}
	}
	
	public String alignUttTemplates(ArrayList<String> inputUtt,
			                     String inputText,
			                     String pruning,
			                     boolean showResult) {
		
		// make the feat for the input
		double[][] input_mat = convertUtt2Mat(inputUtt, doDelta);
		
		int m = templateVecSize; int n = inputUtt.size();
	    //DTW matrix
		double[][] dtwDist = new double[m][n];
	    int t, i, j;
	    
        // initialize the DTW matrix and backpointers
		for (t = 0; t < templateNum; t++) {
			int templateX = templateIndexes[t];
			int templateLen = templateLengths[t];
			dtwDist[templateX][0] = euclideanDistance(input_mat[0], template_mat[templateX]);
			for (j = 1; j < n; j++) {
				dtwDist[templateX][j] = dtwDist[templateX][j-1]
				        + euclideanDistance(input_mat[j], template_mat[templateX]);
			}
			for (i = templateX + 1; i < templateX + templateLen; i++) {
				dtwDist[i][0] = dtwDist[i-1][0]
				        + euclideanDistance(input_mat[0], template_mat[i]);
			}
		}
			
		// indicator about whether the current template is active   1: active   0: inactive
		int[] templateStates = new int[templateNum];
		for (t = 0; t < templateNum; t++) templateStates[t] = 1;
		//compute the DTW matrix
		for (j = 1; j < n; j++) {
		  double min_value = Double.MAX_VALUE;
		  for (t = 0; t < templateNum; t++) {
			  if (templateStates[t] == 0) continue;
			  int templateX = templateIndexes[t];
			  int templateLen = templateLengths[t];
			  for (i = templateX + 1; i < templateX + templateLen; i++) {
				  double dist = euclideanDistance(input_mat[j], template_mat[i]);
				  double min_cost = dtwDist[i - 1][j]; 
				  if (dtwDist[i - 1][j - 1] >= 0 && min_cost > dtwDist[i - 1][j - 1])
					  min_cost = dtwDist[i - 1][j - 1];
				  if (dtwDist[i][j - 1] >= 0 && min_cost > dtwDist[i][j - 1])
					  min_cost = dtwDist[i][j - 1];
				  dtwDist[i][j] = min_cost + dist;
				  // get the min value of the whole column
				  if (min_value > dtwDist[i][j]) min_value = dtwDist[i][j];
				} // end of i
			} // end of t
				    
			// beam pruning
		    // we don't do pruning on the last column
			if (pruning.equals("none") || j == (n - 1)) continue;
				
		    int total_num_above_zero = 0;
			for (t = 0; t < templateNum; t++) {
				if (templateStates[t] == 0) continue;
			  	int num_above_zero = 0;
			  	int templateX = templateIndexes[t];
				int templateLen = templateLengths[t];
				for (i = templateX + 1; i < templateX + templateLen; i++) {
					if (pruning.equals("beam") && dtwDist[i][j] > (min_value + beam_prune_thresh)) {
						dtwDist[i][j] = -1;
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
				System.err.println("All the template have died, due to pruning    {*ERROR*}");
				return "ERROR";
			}
				
		} // end of j
			
		// Now we need to find the best score
		int bestTemplateIndex = -1; double bestDtwScore = Double.MAX_VALUE;
		for (t = 0; t < templateNum; t++) {
			if (templateStates[t] == 0) continue;
			double thisDtwScore = dtwDist[templateIndexes[t] + templateLengths[t] - 1][n - 1];				
			if (bestDtwScore > thisDtwScore && thisDtwScore >= 0) {
				bestDtwScore = thisDtwScore;
				bestTemplateIndex = t;
			}
		}
		
		if (bestTemplateIndex == -1) {
			System.err.println("The input cannot be matched"
					+ "against the templates, likely due to pruning    {*ERROR*}");
			return "ERROR";
		}
		
		String recResult = templateTexts.get(bestTemplateIndex);
		String marker = "";
		if (!recResult.equals(inputText) && recResult != inputText) {
			marker = "{*ERROR*}";
		}
		
		if (showResult) {
		    System.out.println("Best match of [" + inputText + "] is ["
				+ recResult + "] with distance = "
				+ dtwDist[templateIndexes[bestTemplateIndex] + templateLengths[bestTemplateIndex] - 1][n - 1]
				+ "    " + marker);
		}
		
		return recResult;
	}
	
	/**
	 * Compute the Euclidean Distance between vectors
	 */
	private double euclideanDistance(double[] vec1, double[] vec2) {
		int dim = vec1.length;
		double dist = 0;
		for (int d = 0; d < dim; ++d) {
	        dist += Math.pow((vec1[d] - vec2[d]), 2);
		}
		return Math.sqrt(dist);
	}
	
	/**
	 * 
	 * @param args
	 */
	private double[][] convertUtt2Mat(ArrayList<String> utterance,
			                          boolean doDelta) {
		int featNum = utterance.size();
		double [][] feat_mat = new double[featNum][3 * raw_mfcc_dim];
		double [][] raw_mat = new double[featNum][raw_mfcc_dim];
		for (int i = 0; i < featNum; i++) {
			String[] strArray = utterance.get(i).split(" ");
		    for (int d = 0; d < raw_mfcc_dim; d++) {
		    	feat_mat[i][d] = Double.parseDouble(strArray[d]);
		    	raw_mat[i][d] = Double.parseDouble(strArray[d]);
		    }
		}
		
		if (!doDelta) return raw_mat;
		
		double[][] delta_feat_mat = computeDelta(raw_mat);
		double[][] dbl_delta_feat_mat = computeDelta(delta_feat_mat);
		for (int i = 0; i < featNum; i++) {
			for (int d = raw_mfcc_dim; d < 2 * raw_mfcc_dim; d++) {
				feat_mat[i][d] = delta_feat_mat[i][d - raw_mfcc_dim];
			}
			for (int d = 2 * raw_mfcc_dim; d < 3 * raw_mfcc_dim; d++) {
				feat_mat[i][d] = dbl_delta_feat_mat[i][d - 2*raw_mfcc_dim];
			}
		}
		return feat_mat;
	}
	
	/**
	 * Compute the delta version of a double mat
	 * @param 
	 * @return
	 */
    private double[][] computeDelta(double[][] mat) {
    	
		int dimM = mat.length; int dimN = mat[0].length;
		double[][] mat_delta = new double[dimM][dimN];
    	int w = 0, left = 0, right = 0;
		
    	for (int m = 0; m < dimM; m++) {
    		double normalizer = 0;
    		for (int n = 0; n < dimN; n++) {
    			mat_delta[m][n] = 0;
    		}
    	    for (w = 1; w <= delta_window; w++) {
    		    left = m - w; right = m + w;
    		    if (left < 0) left = 0;
    		    if (right > (dimM - 1)) right = dimM - 1;
    		    for (int n = 0; n < dimN; n++) {
    		    	mat_delta[m][n] += w * (mat[right][n] - mat[left][n]);
    		    }
    		    normalizer += w * w;
    	    }
    	    normalizer *= 2;
    	    for (int n = 0; n < dimN; n++) mat_delta[m][n] /= normalizer;
    	}
    	return mat_delta;
	}
	
	public static void main(String args[]) {
    
//		UtteranceMatching uttMatching = new UtteranceMatching();
//		ArrayList<String> list = new ArrayList<String>();
//		list.add("12.256 -1.418 0.344 -0.248 -0.334 0.131 -0.166 -0.181 -0.175 -0.008 -0.176 -0.129 -0.106");
//		list.add("10.463 -0.056 -0.024 -0.152 -0.046 -0.130 -0.003 -0.170 -0.168 -0.070 -0.152 -0.166 -0.095");
//		list.add("10.419 -0.202 -0.107 -0.068 -0.055 -0.155 -0.054 -0.090 -0.163 -0.085 -0.208 -0.180 -0.105");
//		list.add("10.213 -0.393 -0.209 -0.183 -0.175 -0.303 -0.145 0.025 -0.102 0.057 0.005 -0.149 -0.109");
//		list.add("10.131 -0.334 -0.133 -0.103 -0.100 -0.246 -0.190 -0.168 -0.169 -0.143 0.046 -0.181 -0.244");
//		list.add("10.247 -0.295 -0.128 -0.174 -0.156 -0.238 -0.111 -0.042 -0.052 -0.016 -0.045 -0.197 0.059");
//		list.add("10.170 -0.218 -0.136 0.045 -0.154 -0.243 -0.119 0.100 0.093 -0.085 -0.035 -0.211 -0.037");
//		list.add("10.037 -0.249 -0.102 0.049 -0.126 -0.163 0.002 0.056 -0.056 -0.067 -0.018 -0.199 -0.081");
//		list.add("9.955 -0.267 -0.088 -0.082 -0.028 -0.119 -0.163 0.003 0.110 0.092 -0.096 -0.263 -0.002");
//		uttMatching.convertUtt2Mat(list);
	}
}
