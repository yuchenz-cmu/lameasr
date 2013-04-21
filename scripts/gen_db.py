#!/usr/bin/env python
# Generates database for training

import os
import sys

num_list = ['zero', 'one', 'two', 'three', 'four', 'five', 'six', 'seven', 'eight', 'nine']

def gen_topo(trans_text, sil_word):
    node_idx = 0
    trans_tokens = trans_text.split()
    topo_str = ""
    topo_str += "%d\n"%(len(trans_tokens) + 1)
    topo_str += "0\n%d\n"%(len(trans_tokens))
    topo_str += "#\n"
    
    for item in trans_tokens:
        topo_str += "%d %d %s\n"%(node_idx, node_idx, sil_word)
        topo_str += "%d %d %s\n#\n"%(node_idx, node_idx + 1, item)
        node_idx += 1

    return topo_str
    

def main():
    if (len(sys.argv) != 4):
        sys.stderr.write("Usage: ./gen_db.py <feat_folder> <db_file> <topo_folder>\n")
        sys.exit(2)

    feat_folder = os.path.abspath(sys.argv[1])
    db_file = sys.argv[2]
    topo_folder = os.path.abspath(sys.argv[3])

    fdb = open(db_file, 'w')
    feat_file_list = os.listdir(feat_folder)
    utt_prefix = ""
    utt_idx = 0

    for feat_file in feat_file_list:
        tokens = feat_file.split("_")
        trans_digits = tokens[0]
        trans_text = ""
        for i in xrange(len(trans_digits)):
            trans_text += num_list[int(trans_digits[i])] + " "

        trans_text = trans_text.strip()
        utt_id = utt_prefix + str(utt_idx)
        topo_file = os.path.join(topo_folder, utt_id + ".topo")
        fdb.write("%s\t%s\t%s\t%s\n"%(utt_id, trans_text, topo_file, os.path.join(feat_folder, feat_file)))

        topo_str = gen_topo(trans_text, "sil_2")
        ftopo = open(topo_file, 'w')
        ftopo.write(topo_str)
        ftopo.close()

        utt_idx += 1

    fdb.close()

if __name__ == "__main__":
    main()

