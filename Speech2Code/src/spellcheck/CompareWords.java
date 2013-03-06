package spellcheck;
import java.io.BufferedReader;
import java.io.FileReader;
import java.io.IOException;
import java.util.ArrayList;

import spellcheck.StringMatching;

public class CompareWords {
	
	/**
	 * @param args
	 */
	public static void main(String[] args) {
		// here you can specify the two words you want to compare
        String inputWord = "Eleaphent";
        String templateWord = "Elephant";
        // pruning has three options: "none", "max", "beam"
        String pruning = "max";
        // perform actual alignment
        StringMatching strMatching = new StringMatching();
        strMatching.alignTwoWords(inputWord, templateWord, pruning);
	}

}
