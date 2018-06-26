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

#ifndef SINSY_PHONEME_INFO_H_
#define SINSY_PHONEME_INFO_H_

#include <string>

namespace sinsy
{

class PhonemeInfo
{
public:
   static const std::string TYPE_SILENT;
   static const std::string TYPE_PAUSE;
   static const std::string TYPE_VOWEL;
   static const std::string TYPE_CONSONANT;
   static const std::string TYPE_BREAK;


   //! constructor
   PhonemeInfo();

   //! constructor
   PhonemeInfo(const std::string& type, const std::string& phoneme, bool e = true, bool f = false, bool stress = false, int wn =0);

   //! copy constructor
   PhonemeInfo(const PhonemeInfo& obj);

   //! destructor
   virtual ~PhonemeInfo();

   //! assignment operator
   PhonemeInfo& operator=(const PhonemeInfo&);

   //! get type
   const std::string& getType() const;

   //! get phoneme
   const std::string& getPhoneme() const;

   //! get phoneme
   //const int setPhoneme(std::string p);

   //! get stress
   const bool isStressed() const;

   //! get wordnum
   const int getWordnum() const;

   //! return which enable or not
   bool isEnable() const;

   //! return which falsetto or not
   bool isFalsetto() const;

private:
   //! type (TYPE_SILENT, TYPE_PAUSE, ..., or TYPE_BREAK)
   std::string type;

   //! phoneme
   std::string phoneme;

   //! flag of helpful or not for training
   bool enableFlag;

   //! flag of falsetto
   bool falsettoFlag;

   bool stress;

   int wordnum;
};

};

#endif // SINSY_PHONEME_INFO_H_
