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

#include <algorithm>
#include <stdexcept>
#include "Dynamics.h"
#include "util_log.h"
#include "util_string.h"

using namespace sinsy;

namespace
{
const int DYNAMICS_NUM = 11;



const std::string STR_P4 = "p4";
const std::string STR_P3 = "p3";
const std::string STR_P2 = "p2";
const std::string STR_P1 = "p1";
const std::string STR_MP = "mp";
const std::string STR_N = "n";
const std::string STR_MF = "mf";
const std::string STR_F1 = "f1";
const std::string STR_F2 = "f2";
const std::string STR_F3 = "f3";
const std::string STR_F4 = "f4";
const std::string STR_PPPP = "pppp";
const std::string STR_PPP = "ppp";
const std::string STR_PP = "pp";
const std::string STR_P = "p";
const std::string STR_F = "f";
const std::string STR_FF = "ff";
const std::string STR_FFF = "fff";
const std::string STR_FFFF = "ffff";

const std::string DYNAMICSES[] = {
   STR_P4, STR_P3, STR_P2, STR_P1, STR_MP, STR_N, STR_MF, STR_F1, STR_F2, STR_F3, STR_F4
};
const std::string DYNAMICS_TAGS[] = {
   STR_PPPP, STR_PPP, STR_PP, STR_P, STR_MP, STR_N, STR_MF, STR_F, STR_FF, STR_FFF, STR_FFFF
};

class Comp
{
public:
   //! constructor
   Comp(const std::string& str) : target(str) {}

   //! destructor
   virtual ~Comp() {}

   //! ...
   bool operator()(const std::string& str) const {
      return (0 == target.compare(str)) ? true : false;
   }

   //! target
   const std::string& target;
};

};

const Dynamics Dynamics::PPPP(STR_PPPP);
const Dynamics Dynamics::PPP(STR_PPP);
const Dynamics Dynamics::PP(STR_PP);
const Dynamics Dynamics::P(STR_P);
const Dynamics Dynamics::MP(STR_MP);
const Dynamics Dynamics::N(STR_N);
const Dynamics Dynamics::MF(STR_MF);
const Dynamics Dynamics::F(STR_F);
const Dynamics Dynamics::FF(STR_FF);
const Dynamics Dynamics::FFF(STR_FFF);
const Dynamics Dynamics::FFFF(STR_FFFF);

/*!
 constructor
 */
Dynamics::Dynamics() : value((DYNAMICS_NUM - 1) / 2)
{
}

/*!
 constructor
 */
Dynamics::Dynamics(const std::string& str)
{
   set(str);
}

/*!
 copy constructor
 */
Dynamics::Dynamics(const Dynamics& obj) : value(obj.value)
{
}

/*!
 destructor
 */
Dynamics::~Dynamics()
{
}

/*!
 assignment operator
 */
Dynamics& Dynamics::operator=(const Dynamics & obj)
{
   value = obj.value;
   return *this;
}

/*!
 equal
 */
bool Dynamics::operator==(const Dynamics& obj) const
{
   return (obj.value == value);
}

/*!
 not equal
 */
bool Dynamics::operator!=(const Dynamics& obj) const
{
   return !(obj == *this);
}

/*!
 set value
 */
void Dynamics::set(const std::string& _str)
{
   std::string s(_str);
   toLower(s);
   /*Sudden changes in dynamics may be notated by adding the word subito (Italian for suddenly)
   as a prefix or suffix to the new dynamic notation. Accented notes (notes to emphasize or
   play louder compared to surrounding notes) can be notated sforzando, sforzato, forzando or
   forzato (abbreviated sfz or fz) ("forcing" or "forced"). One particularly noteworthy use
   of forzando is in the second movement of Joseph Haydn's Surprise Symphony.
   We encode this as forte or fforte.
    <xs:element name="sf" type="empty"/>
    <xs:element name="sfz" type="empty"/>
    <xs:element name="fz" type="empty"/>
    <xs:element name="sffz" type="empty"/>*/
   if(s.compare("fz") || s.compare("sfz") || s.compare("f")){
	   s = "f";
   }else if(s.compare("sffz")){
	   s = "ff";
   }
   /*
   The fortepiano notation fp indicates a forte followed immediately by piano.
   Sforzando piano (sfzp or sfp) indicates a sforzando followed immediately by piano; in general,
   any two dynamic markings may be treated similarly.
   We encode this as normal since it can only be defined contextually
   <xs:element name="fp" type="empty"/>
   <xs:element name="sfp" type="empty"/>
   <xs:element name="sfpp" type="empty"/>
   Rinforzando, rfz or rf (literally "reinforcing") indicates that several notes, or a short phrase,
   are to be emphasized.
   We encode this as normal since it can only be defined contextually
   <xs:element name="rf" type="empty"/>
   <xs:element name="rfz" type="empty"/>
   */
   else if(s.compare("fp") || s.compare("sfp") || s.compare("sfpp") || s.compare("rf") || s.compare("rfz")){
   	   s = "n";
   }
   /*<xs:element name="ppppp" type="empty"/>
    <xs:element name="pppppp" type="empty"/>
    <xs:element name="fffff" type="empty"/>
    <xs:element name="ffffff" type="empty"/>
    Remaining dynamics are set to pppp and ffff.
    */
   else if(s.compare("ppppp") || s.compare("pppppp")){
      s = "pppp";
   }else if(s.compare("fffff") || s.compare("ffffff")){
      s = "ffff";
   }


   const std::string* itr(std::find_if(DYNAMICSES, DYNAMICSES + DYNAMICS_NUM, Comp(s)));
   if (itr < DYNAMICSES + DYNAMICS_NUM) {
      value = itr - DYNAMICSES;
      return;
   }
   itr = std::find_if(DYNAMICS_TAGS, DYNAMICS_TAGS + DYNAMICS_NUM, Comp(s));
   if (itr < DYNAMICS_TAGS + DYNAMICS_NUM) {
      value = itr - DYNAMICS_TAGS;
      return;
   }
   ERR_MSG("Unexpected dynamics : " << s);
   throw std::runtime_error("Dynamics::set() invalid argument");
}

/*!
 get valie as string
 */
const std::string& Dynamics::getStr() const
{
   return DYNAMICSES[value];
}

/*!
 get tag string
 */
const std::string& Dynamics::getTagStr() const
{
   return DYNAMICS_TAGS[value];
}

/*!
 to string
 */
std::ostream& sinsy::operator<<(std::ostream& os, const Dynamics& dynamics)
{
   return os << dynamics.getStr();
}
