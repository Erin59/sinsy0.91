/**
    GConf.cpp
    Purpose: Reads in German lexicon and phones, collects syllables from the language
    part of musicXML and finds the words in the lexicon, writing phone information for the syllables back

    @author Michael Pucher FTW/NII 2014
*/

/* ----------------------------------------------------------------- */
/*           The HMM-Based Singing Voice Synthesis System "Sinsy"    */
/*           developed by Sinsy Working Group                        */
/*           http://sinsy.sourceforge.net/                           */
/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2009-2013  Nagoya Institute of Technology          */
/*                           Department of Computer Science          */
/*                                                                   */
/* All rights reserved.                                              */
/*                                                                   */
/* Redistribution and use in source and binary forms, with or        */
/* without modification, are permitted provided that the following   */
/* conditions are met:                                               */
/*                                                                   */
/* - Redistributions of source code must retain the above copyright  */
/*   notice, this list of conditions and the following disclaimer.   */
/* - Redistributions in binary form must reproduce the above         */
/*   copyright notice, this list of conditions and the following     */
/*   disclaimer in the documentation and/or other materials provided */
/*   with the distribution.                                          */
/* - Neither the name of the Sinsy working group nor the names of    */
/*   its contributors may be used to endorse or promote products     */
/*   derived from this software without specific prior written       */
/*   permission.                                                     */
/*                                                                   */
/* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND            */
/* CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,       */
/* INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF          */
/* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE          */
/* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS */
/* BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,          */
/* EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED   */
/* TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,     */
/* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON */
/* ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,   */
/* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY    */
/* OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE           */
/* POSSIBILITY OF SUCH DAMAGE.                                       */
/* ----------------------------------------------------------------- */

#include <stdexcept>
#include <limits>
#include <deque>
#include <vector>
#include "util_log.h"
#include "util_string.h"
#include "util_converter.h"
#include "StringTokenizer.h"
#include "GConf.h"
#include "Deleter.h"
#include <ostream>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <iterator>
#include <vector>
#include <sstream>
#include "string.h"
#include "ctype.h"
#include "util_converter.h"
#include "XmlReader.h"
#include "XmlWriter.h"
#include "InputFile.h"
#include "OutputFile.h"
#include "WritableStrStream.h"
#include "sinsy.h"
#include "NoteLabeler.h"
#include "LTSTree.h"

using namespace sinsy;

namespace
{
const std::string SIL_STR = "sil";
const std::string PAU_STR = "pau";
//used for convertable elements that have no lyrics
const std::string PAU1_STR = "pau1";
const char DISABLE_CHAR = '#';
const char FALSETTO_CHAR = '$';
const std::string LANGUAGE_INFO = "DE";
//Syllables are separated by this string in a standard Festival lexicon
const std::string SYLLABLE_SEPARATOR = ") (";
//A vector containing the German vowels
std::vector<std::string> vowels;

std::vector<std::string> words;

//A map containing the German lexicon
std::map <std::string, std::string> lexicon;
//The LTS tree structure
LTSTree lts;
//Mapping between German and Japanese phones, only used if phone_mapping=true
std::map <std::string, std::string> phone_mapping;
//Set this to true if you want to use a German XML file and a Japanese acoustic model
const bool map_german_to_japanese = false;


class Syllable {
public:
	/*! constructor
	 @param syl string with syllable
	 @param enableFlag passed on, not handled at the moment
	 @param falsettoFlag passed on, not handled at the moment
	 @param stressed syllable
	 */
	Syllable(std::string syl,bool enableFlag=true, bool falsettoFlag=false,bool stress=false,int wordnum=1){
		stressed = stress;
	    StringTokenizer st(syl, " ",true);
		for(int i=0;i<st.size();i++){
			std::string phon = st.at(i);
		    if(!phon.empty() && !(phon=="\n")){
		    	phon.erase(std::remove(phon.begin(), phon.end(), ' '), phon.end());
		    	std::string ptype;
		    	if(std::find(vowels.begin(), vowels.end(), phon)!=vowels.end()){
		    		ptype = PhonemeInfo::TYPE_VOWEL;
		    	}else{
		    		ptype = PhonemeInfo::TYPE_CONSONANT;
		    	}
    			if(map_german_to_japanese){
    				phonemes.push_back(PhonemeInfo(ptype, phone_mapping.at(phon), enableFlag, falsettoFlag,stress,wordnum));
		    	}else{
		    		phonemes.push_back(PhonemeInfo(ptype, phon, enableFlag, falsettoFlag,stress,wordnum));
		    	}
    		}
    	}
	}

	/*! desctructor
	 *
	 */
	virtual ~Syllable(){
		phonemes.clear();
	}

	/*!
	 * return a pointer to the PhonemeInfo vector
	 */
	std::vector<PhonemeInfo> getPhonemeInfoVec(){
		return phonemes;

	}

	/*! is it a stressed syllable?
	 *
	 */
	bool isStressed(){
		return stressed;
	}

private:
	std::vector<PhonemeInfo> phonemes;
	bool stressed;

};


class Word {
public:
	/*! constructor
	 @param word string with word
	 @param enableFlag passed on, not handled at the moment
	 @param falsettoFlag passed on, not handled at the moment
	 @param pos part-of-spech flag, default N for noun
	 */

	Word(const std::string word,std::vector<std::string> &wordvec, bool enableFlag=true, bool falsettoFlag=false,std::string pos="N",int wordnum=1){
		//std::cout << word << std::endl;
		ortho = word;
		postag = pos;
		std::cout << word << "\n";
     	std::map<std::string,std::string>::iterator it;
		it=lexicon.find(word);
		if(it!=lexicon.end()){
			std::string syl_phones = lexicon.at(word);
			//split into syllables and pass to syllable class
		    StringTokenizer st(syl_phones, ") (",true);
		    for(int i=0;i<st.size();i++){
		    	std::string sylstr = st.at(i);
				size_t idx = sylstr.find("(",0);
				sylstr = sylstr.substr(idx,sylstr.length());

				bool stress = false;
				std::size_t found = sylstr.find("1");
				  if (found!=std::string::npos)
					  stress = true;

				sylstr.replace(sylstr.length()-2,sylstr.length()-1,"");
				sylstr.erase(std::remove(sylstr.begin(), sylstr.end(), '('), sylstr.end());
				sylstr.erase(std::remove(sylstr.begin(), sylstr.end(), ')'), sylstr.end());
				sylstr.erase(std::remove(sylstr.begin(), sylstr.end(), '0'), sylstr.end());
				sylstr.erase(std::remove(sylstr.begin(), sylstr.end(), '1'), sylstr.end());
				sylstr.erase(std::remove(sylstr.begin(), sylstr.end(), '\n'), sylstr.end());
		    	Syllable syl = Syllable(sylstr,enableFlag,falsettoFlag,stress,wordnum);
		    	syllable_vec.push_back(syl);
		    }
		}else{

			for (int i=0; i < wordvec.size(); i++) {
				//use LTS to predict word
				std::string wordstr = "";

				sinsy::utt::WordPtr wordptr = lts[wordvec[i]];
				 for (sinsy::utt::SyllablePtr sylptr : wordptr->syllables) {
					 for (sinsy::utt::PhonePtr phoneptr : sylptr->phones) {
						 wordstr = wordstr + phoneptr->symbol + " ";
		            }
		         }
				//std::cout << wordstr << "\n";
		    	Syllable syl = Syllable(wordstr,enableFlag,falsettoFlag,false,wordnum);
		    	syllable_vec.push_back(syl);
			}

			WARN_MSG(word + " not found in lexicon, used LTS!");

		}

	}

	/*! destructor
	 *
	 */
	virtual ~Word(){
		syllable_vec.clear();
	}



	/*!
	 * return a pointer to the syllable vector syllable_vec
	 */
	std::vector<Syllable> getSyllableVec(){
		return syllable_vec;

	}

	/*!
	 * what's the part-of-speech (POS) tag of the word
	 */
	std::string getPOSTag(){
		return postag;
	}

	std::string ortho;
	std::vector<Syllable> syllable_vec;
	std::string postag;
};

};


/*!
 constructor
 */
GConf::GConf()
{

}

/*!
 destructor
*/
GConf::~GConf()
{
}


bool GConf::printWords(const std::string& filename) const
{
	std::ofstream labfile;
	std::string newfn = filename + ".data";
	//std::cout << newfn << std::endl;
	labfile.open (newfn.c_str());

	int i;
	for (i=0; i < words.size()-1; i++) {
		//std::cout << stringList[i] << '\n';
		labfile << words[i] << " ";
	}
	labfile << words[i] << ".\n";

	labfile.close();

	return true;
}



/*!
 @param lexiconn name of lexicon file
 @param vowelsn name of vowels file
 @param mapfilen mapping between German and Japanese vowels
 @return true if success
 */
bool GConf::read(const std::string& lexiconn, const std::string& vowelsn, const std::string& mapfilen, const std::string& ltsrules)
{

	if(map_german_to_japanese){
		std::string line1;
		std::ifstream phonemapfile(mapfilen.c_str());
		if (phonemapfile.is_open()){
			while ( getline (phonemapfile,line1) ){
				std::size_t idx = line1.find(" ");
				std::string gphone = line1.substr(0,idx);
				std::string jphone = line1.substr(idx+1,line1.length()-2-gphone.length());
				if (!jphone.empty() && jphone[jphone.length()-1] == '\n') {
					jphone.erase(jphone.length()-1);
				}
				jphone.erase(std::remove(jphone.begin(), jphone.end(), '\n'), jphone.end());
				phone_mapping.insert( std::pair<std::string, std::string>(gphone,jphone) );
			}
			phonemapfile.close();
		}else{
			ERR_MSG("Cannot open Mapping file: " +mapfilen);
		}
	}

	std::string line;
	std::ifstream phonesfile(vowelsn.c_str());
	if (phonesfile.is_open()){
		while ( getline (phonesfile,line) ){
			vowels.push_back(line);
		}
	    phonesfile.close();
	}else{
		ERR_MSG("Cannot open Vowels file: " << GERMAN_VOWELS);
	}

	std::ifstream lexfile(lexiconn.c_str());
	if (lexfile.is_open()){
		while ( getline (lexfile,line) ){
			std::size_t idx = line.find(" ");
	        std::string word = line.substr(0,idx);
	        std::string phones = line.substr(idx,line.length()-1);
	        std::string word1,word2;

	        word.erase(std::remove(word.begin(), word.end(), '('), word.end());
	        word.erase(std::remove(word.begin(), word.end(), '"'), word.end());

	        lexicon.insert( std::pair<std::string, std::string>(word,phones) );
	    }
	    lexfile.close();
	}else{
		ERR_MSG("Cannot open lexicon file: " << lexiconn);
		return false;
	}

	//Load LTS tree
	std::ifstream myfile(ltsrules.c_str());

	if (myfile.is_open()) {
		lts.Read(myfile);
		myfile.close();
		//std::cout << "LTS loaded" << std::endl;
	}
	else {
		ERR_MSG("Cannot open LTS rules file: " << ltsrules);
		return false;
	}


	return true;
}




/*!
 @param enc encoding
 @param begin begin of vector of IConvertable elements that have to be converted to German
 @param end end of the vector
 @return true if success
*/
bool GConf::convert(const std::string& enc, ConvertableList::iterator begin, ConvertableList::iterator end) const
{

	std::string multisylword = "";
	std::vector<std::string> multisylwordvec;
	bool collect_syl = false;
	int wordnum = 1;

	IConf::ConvertableList::iterator itr(begin);

	std::vector<IConvertable*> ConvertableListMultiSyl;
	ConvertableListMultiSyl.clear();

	for (; itr != end; ++itr) {
		IConvertable& convertable(**itr);
	    //if (!convertable.isConverted()) {
	    	std::string lyric(convertable.getLyric());
        	//std::cout << lyric << std::endl;

	    	lyric.erase(std::remove(lyric.begin(), lyric.end(), '!'), lyric.end());
	    	lyric.erase(std::remove(lyric.begin(), lyric.end(), '?'), lyric.end());
	    	lyric.erase(std::remove(lyric.begin(), lyric.end(), ':'), lyric.end());
	    	lyric.erase(std::remove(lyric.begin(), lyric.end(), ';'), lyric.end());
	    	lyric.erase(std::remove(lyric.begin(), lyric.end(), ','), lyric.end());
	    	lyric.erase(std::remove(lyric.begin(), lyric.end(), '\''), lyric.end());
	    	lyric.erase(std::remove(lyric.begin(), lyric.end(), '.'), lyric.end());
	    	lyric.erase(std::remove(lyric.begin(), lyric.end(), '-'), lyric.end());
	    	lyric.erase(std::remove(lyric.begin(), lyric.end(), ' '), lyric.end());

	    	std::transform(lyric.begin(), lyric.end(), lyric.begin(), ::tolower);

	    	bool enableFlag = true;
	        bool falsettoFlag = false;

	         // check disable char
	        /*for (size_t idx(0); ; ) {
	        	idx = lyric.find(DISABLE_CHAR, idx);
	            if (std::string::npos == idx) {
	               break;
	            }
	            lyric.erase(idx, idx + 1);
	            enableFlag = false;
	        }

	         // check falsetto char
	        for (size_t idx(0); ; ) {
	        	idx = lyric.find(FALSETTO_CHAR, idx);
	            if (std::string::npos == idx) {
	               break;
	            }
	            lyric.erase(idx, idx + 1);
	            falsettoFlag = true;
	        }*/

	        if(lyric!=""){
	        	//end of word with multiple syllables
	        	if (convertable.getSyllabic()==convertable.getSyllabic().END){

	        		multisylword = multisylword +  lyric;
	        		multisylwordvec.push_back(lyric);

	        		//add all phones syllablewise to convertable
	        		ConvertableListMultiSyl.push_back(&convertable);
	        		Word w = Word(multisylword,multisylwordvec,enableFlag,falsettoFlag,"N",wordnum);
	        		words.push_back(multisylword);
	        		wordnum = wordnum + 1;

	        		if(ConvertableListMultiSyl.size()!=w.getSyllableVec().size()){
	        			ERR_MSG(multisylword +": Different number of syllables in musicXML and German lexicon");
	        		}

	        		int idx = 0;

	        		for (IConf::ConvertableList::iterator it = ConvertableListMultiSyl.begin() ; it != ConvertableListMultiSyl.end(); ++it){
	        			IConvertable& convertable1(**it);
	        			std::vector<PhonemeInfo> phonemes = w.getSyllableVec().at(idx).getPhonemeInfoVec();
	        			convertable1.addInfo(phonemes, LANGUAGE_INFO, "");
	        			idx++;

	        		}
	        		//remove all elements
	        		ConvertableListMultiSyl.clear();
	        		multisylword = "";
	        		multisylwordvec.clear();
	        		collect_syl = false;

	        	//should be a word with one syllable, so go
	        	}else if(convertable.getSyllabic()==convertable.getSyllabic().SINGLE && !collect_syl){

	        		multisylwordvec.clear();
	        		multisylwordvec.push_back(lyric);

	        		Word w = Word(lyric,multisylwordvec,falsettoFlag,falsettoFlag,"N",wordnum);
	        		wordnum = wordnum + 1;
	        		words.push_back(lyric);
	        		Syllable syl = w.getSyllableVec().front();
	        		std::vector<PhonemeInfo> phonemes = syl.getPhonemeInfoVec();
	        		convertable.addInfo(phonemes, LANGUAGE_INFO, "");

	        	}else if (convertable.getSyllabic()==convertable.getSyllabic().BEGIN){

	        		collect_syl = true;
	        		multisylword = lyric;
	        		multisylwordvec.clear();
	        		multisylwordvec.push_back(lyric);
	        		//remember where we start a sequence
	        		ConvertableListMultiSyl.push_back(&convertable);

	        	//Should have an error here, but most XML files are wrong
	        	}else if (convertable.getSyllabic()==convertable.getSyllabic().SINGLE && collect_syl){

	        		multisylword = multisylword +  lyric;
	        		multisylwordvec.push_back(lyric);
	        		ConvertableListMultiSyl.push_back(&convertable);

	        	}else if (convertable.getSyllabic()==convertable.getSyllabic().MIDDLE){

	        		multisylword = multisylword +  lyric;
	        		multisylwordvec.push_back(lyric);
	        		ConvertableListMultiSyl.push_back(&convertable);

	        	}
	        //}else if (collect_syl){//lyric is empty, we still do not want to add pauses within words collect_syl
	        	//find last phone and copy it
	        	/*IConvertable& convertable1(**ConvertableListMultiSyl.end());
	        	((NoteLabeler)convertable1).
    			std::vector<PhonemeInfo> phonemes = w.getSyllableVec().at(idx).getPhonemeInfoVec();
  	        	convertable.addInfo(phonemes, LANGUAGE_INFO, "");*/

  	        	//remove last phone from previous convertable

	        //}
	        }else {//lyric is empty, we still do not want to add pauses within words !collect_syl
	        	//add pau symbol for notes, set enabled flag to false
	        	std::vector<PhonemeInfo> phonemes;
	        	phonemes.push_back(PhonemeInfo(PhonemeInfo::TYPE_PAUSE, PAU1_STR, false, false));
	        	convertable.addInfo(phonemes, LANGUAGE_INFO, "");
	        }
	    //}
	}//end for

	//IConf::ConvertableList::iterator itr1(begin);


	//for (; itr1 != end; ++itr1) {
	//	IConvertable& convertable(**itr1);
	//	if(((NoteLabeler*)convertable)->isSlurStart()){
			//std:cout << ((NoteLabeler)convertable).getSyllabic() << " ";
	//	}

	//}

	return true;

}

/*!
  return sil str
 */
std::string GConf::getSilStr() const
{
   return SIL_STR;
}



