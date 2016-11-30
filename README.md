# CATTI
Computer Assisted Transcription of Handwritten Text Images

The software can be compiles as a shared library (catti-plugin.so). 

Compilation:

cd wg/lat-parser/
make
cd ..
make
cd ..
make

This plugin requires a configuration file with two parameters. The first one (-W) specifies the name of a file with the correspondence between the ids and the wordgraphs. The second one (-G) is an unigram in HTK format. 

In catti-example.cpp you can see an example about how to use the plugin. Given a list of wordgraph ids this example obtain the best transcription.

./catti-example Test/test

The two main functions of the plugin are:

-setPrefix: The user validates a prefix and introduce some correction. The system returns a new suffix taking into account the corrections carried out by the user.
Parameters:
           -const vector<string> &prefix: The validated prefix with the user correction. If the prefix is empty the plugin returns the most probably transcription.
           -const vector<string> &suffix: The rejected suffix.
           -const bool last_token_is_partial: It is false if the last word of the prefix is a complete word.
           -vector<string> &corrected_transcription: It is the new transcription.

-rejectSuffix:The user validates a prefix and no introduce any correction. The system returns the most probably suffix different to the previous one. Has the same parameters than the setPrefix function. 
