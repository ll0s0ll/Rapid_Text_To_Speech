COMPILE.cc = $(CXX) $(CXXFLAGS) $(CPPFLAGS) $(TARGET_ARCH)

# コンパイラフラグの指定がない場合は、最適化しちゃう。
ifndef CXXFLAGS
 CXXFLAGS += -O2 -w
endif 

ifndef OJTDIR
 OPENJTALK_DIR := 
# OPENJTALK_DIR := /Users/admin/Downloads/open_jtalk-1.05_mac/
else
 OPENJTALK_DIR := $(OJTDIR)
endif

ifndef HTSEDIR
 HTS_ENGINE_DIR := 
# HTS_ENGINE_DIR := /Users/admin/Downloads/hts_engine_API-1.06_mac
else
 HTS_ENGINE_DIR := $(HTSEDIR)
endif

# OSがMACOSX指定の場合はマクロを追加する。
ifeq "$(OS)" "MACOSX"
 CXXFLAGS += -DMACOSX
endif

## LIBS

# HTSEngine
INCLUDES += -I$(HTS_ENGINE_DIR)/include/
LIBS += -L$(HTS_ENGINE_DIR)/lib -lHTSEngine

# mecab
INCLUDES += -I$(OPENJTALK_DIR)/mecab/src
LIBS += -L$(OPENJTALK_DIR)/mecab/src -lmecab

# MacOSXで使用する場合は、iconvをリンクする。iconvはmecabで使用する。
ifeq "$(OS)" "MACOSX"
 LIBS += -liconv
endif

# NJD
INCLUDES += -I$(OPENJTALK_DIR)/njd 
LIBS += -L$(OPENJTALK_DIR)/njd -lnjd

# JPCommon
INCLUDES += -I$(OPENJTALK_DIR)/jpcommon
LIBS += -L$(OPENJTALK_DIR)/jpcommon -ljpcommon

# text2mecab
INCLUDES += -I$(OPENJTALK_DIR)/text2mecab
LIBS += -L$(OPENJTALK_DIR)/text2mecab  -ltext2mecab

# mecab2njd
INCLUDES += -I$(OPENJTALK_DIR)/mecab2njd
LIBS += -L$(OPENJTALK_DIR)/mecab2njd -lmecab2njd

# njd_set_pronunciation
INCLUDES += -I$(OPENJTALK_DIR)/njd_set_pronunciation
LIBS += -L$(OPENJTALK_DIR)/njd_set_pronunciation -lnjd_set_pronunciation

# njd_set_digit
INCLUDES += -I$(OPENJTALK_DIR)/njd_set_digit
LIBS += -L$(OPENJTALK_DIR)/njd_set_digit -lnjd_set_digit

# njd_set_accent_phrase
INCLUDES += -I$(OPENJTALK_DIR)/njd_set_accent_phrase
LIBS += -L$(OPENJTALK_DIR)/njd_set_accent_phrase -lnjd_set_accent_phrase

# njd_set_accent_type
INCLUDES += -I$(OPENJTALK_DIR)/njd_set_accent_type
LIBS += -L$(OPENJTALK_DIR)/njd_set_accent_type -lnjd_set_accent_type

# njd_set_unvoiced_vowel
INCLUDES += -I$(OPENJTALK_DIR)/njd_set_unvoiced_vowel
LIBS += -L$(OPENJTALK_DIR)/njd_set_unvoiced_vowel -lnjd_set_unvoiced_vowel

# njd_set_long_vowel
INCLUDES += -I$(OPENJTALK_DIR)/njd_set_long_vowel
LIBS += -L$(OPENJTALK_DIR)/njd_set_long_vowel -lnjd_set_long_vowel

# njd2jpcommon
INCLUDES += -I$(OPENJTALK_DIR)/njd2jpcommon
LIBS += -L$(OPENJTALK_DIR)/njd2jpcommon -lnjd2jpcommon

# OpenAL (MacOSXで使用する場合はライブラリの指定方法が違う)
ifeq "$(OS)" "MACOSX"
 LIBS += -framework OpenAL
else
 LIBS += -lopenal
endif


RapidTextToSpeech: OpenALSound.cpp RapidTextToSpeech.cpp TextToSpeech.cpp
	$(COMPILE.cc) $^ $(INCLUDES) $(LIBS) -o $@

clean:
	rm RapidTextToSpeech