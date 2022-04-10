QT += sql quick multimedia core
CONFIG += c++17

#CONFIG += AuralsLegacy

AuralsLegacy {
    DEFINES += "AuralsLegacy=\"1\""
}

!win32: QT += webview

#Doesn't work on windows
linux:Debug:CONFIG += sanitizer sanitize_address sanitize_memory sanitize_undefined #sanitize_thread


QMAKE_CXXFLAGS_RELEASE += -O3


#CONFIG += no_keywords #was used only for python interpreter
DEFINES += QT_DEPRECATED_WARNINGS
QMAKE_CXXFLAGS += -std=c++17
QMAKE_CXXFLAGS += -Wall

android: QT += androidextras
android: ANDROID_ABIS = "armeabi-v7a" #
android: ANDROID_ABIS += "arm64-v8a" #

QMAKE_LFLAGS += -v

#Wavelets are commented, yet no application for such amazing detalization

SOURCES += \
    app/AndroidTools.cpp \
    app/Config.cpp \
    app/Init.cpp \
    app/LogHandler.cpp \
    audio/Recorder.cpp \
    audio/features/FeatureExtractor.cpp \
    audio/features/Yin.cpp \
    audio/wave/AudioUtils.cpp \
    diary/CardReport.cpp \
    diary/DiaryCardEngine.cpp \
    diary/SQLBase.cpp \
    diary/TestsEngine.cpp \
    libs/kiss/kfc.c \
    libs/kiss/kiss_fft.c \
    libs/kiss/kiss_fftnd.c \
    libs/kiss/kiss_fftndr.c \
    libs/kiss/kiss_fftr.c \
    main.cpp


AuralsLegacy {

SOURCES += \
    audio/features/ACFgraph.cpp \
    audio/spectrum/Cepstrumgraph.cpp \
    audio/wave/AudioHandler.cpp \
    audio/wave/AudioReceiver.cpp \
    audio/wave/AudioSpeaker.cpp \
    audio/spectrum/FrequencySpectrum.cpp \
    audio/spectrum/Spectrograph.cpp \
    audio/spectrum/SpectrumAnalyser.cpp \
    audio/wave/WavFile.cpp \
    audio/wave/WaveContour.cpp \
    audio/wave/WaveShape.cpp \
    libs/cqt/CQInverse.cpp \
    libs/cqt/CQKernel.cpp \
    libs/cqt/CQSpectrogram.cpp \
    libs/cqt/Chromagram.cpp \
    libs/cqt/ConstantQ.cpp \
    libs/cqt/Pitch.cpp \
    libs/cqt/dsp/FFT.cpp \
    libs/cqt/dsp/KaiserWindow.cpp \
    libs/cqt/dsp/MathUtilities.cpp \
    libs/cqt/dsp/Resampler.cpp \
    libs/cqt/dsp/SincWindow.cpp \
    libs/filters/Biquad.cpp \
    libs/filters/Butterworth.cpp \
    libs/filters/Cascade.cpp \
    libs/filters/ChebyshevI.cpp \
    libs/filters/ChebyshevII.cpp \
    libs/filters/Custom.cpp \
    libs/filters/PoleFilter.cpp \
    libs/filters/RBJ.cpp \
    #libs/wavelet/conv.c \
    #libs/wavelet/cwt.c \
    #libs/wavelet/cwtmath.c \
    #libs/wavelet/hsfft.c \
    #libs/wavelet/real.c \
    #libs/wavelet/wavefilt.c \
    #libs/wavelet/wavefunc.c \
    #libs/wavelet/wavelib.c \
    #libs/wavelet/wtmath.c \

HEADERS += \
    audio/features/ACFgraph.hpp \
    audio/features/PeaksOperations.hpp \
    audio/features/WindowFunction.hpp \
    audio/spectrum/Cepstrumgraph.hpp \
    audio/wave/AudioHandler.hpp \
    audio/wave/AudioReceiver.hpp \
    audio/wave/AudioSpeaker.hpp \
    audio/wave/AudioUtils.hpp \
    audio/spectrum/FrequencySpectrum.hpp \
    audio/spectrum/Spectrograph.hpp \
    audio/spectrum/SpectrumAnalyser.hpp \
    audio/wave/WavFile.hpp \
    audio/wave/WaveContour.hpp \
    audio/wave/WaveShape.hpp \
    libs/cqt/CQBase.h \
    libs/cqt/CQInverse.h \
    libs/cqt/CQKernel.h \
    libs/cqt/CQParameters.h \
    libs/cqt/CQSpectrogram.h \
    libs/cqt/Chromagram.h \
    libs/cqt/ConstantQ.h \
    libs/cqt/Pitch.h \
    libs/cqt/dsp/FFT.h \
    libs/cqt/dsp/KaiserWindow.h \
    libs/cqt/dsp/MathUtilities.h \
    libs/cqt/dsp/Resampler.h \
    libs/cqt/dsp/SincWindow.h \
    libs/cqt/dsp/Window.h \
    libs/cqt/dsp/nan-inf.h \
    libs/cqt/dsp/pi.h \
    libs/fft/FFTRealFixed.hpp \
    libs/filters/Biquad.h \
    libs/filters/Butterworth.h \
    libs/filters/Cascade.h \
    libs/filters/ChebyshevI.h \
    libs/filters/ChebyshevII.h \
    libs/filters/Common.h \
    libs/filters/Custom.h \
    libs/filters/Iir.h \
    libs/filters/Layout.h \
    libs/filters/MathSupplement.h \
    libs/filters/PoleFilter.h \
    libs/filters/RBJ.h \
    libs/filters/State.h \
    libs/filters/Types.h \
    #libs/wavelet/conv.h \
    #libs/wavelet/cwt.h \
    #libs/wavelet/cwtmath.h \
    #libs/wavelet/hsfft.h \
    #libs/wavelet/real.h \
    #libs/wavelet/wauxlib.h \
    #libs/wavelet/wavefilt.h \
    #libs/wavelet/wavefunc.h \
    #libs/wavelet/wavelib.h \
    #libs/wavelet/wtmath.h \

}
else
{
}


RESOURCES += qml.qrc \
    res/cards.qrc \
    res/fonts.qrc \
    res/tests.qrc


QML_IMPORT_PATH =
QML_DESIGNER_IMPORT_PATH =

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target


HEADERS += \
    app/AndroidTools.hpp \
    app/Clipboard.hpp \
    app/Config.hpp \
    app/Init.hpp \
    app/LogHandler.hpp \
    app/StretchImage.hpp \
    app/log.hpp \
    audio/Recorder.hpp \
    audio/features/FeatureExtractor.hpp \
    audio/features/Yin.hpp \
    diary/CardReport.hpp \
    diary/DiaryCardEngine.hpp \
    diary/SQLBase.hpp \
    diary/TestsEngine.hpp \
    libs/kiss/_kiss_fft_guts.h \
    libs/kiss/kfc.h \
    libs/kiss/kiss_fft.h \
    libs/kiss/kiss_fft_log.h \
    libs/kiss/kiss_fftnd.h \
    libs/kiss/kiss_fftndr.h \
    libs/kiss/kiss_fftr.h




DISTFILES += \
    ConsoleLog.qml \
    LICENSE \
    OpenTab.qml \
    README.md \
    qml/SingleRecord.qml \
    qml/calendar.qml \
    qml/config.qml \
    qml/diaryCard.qml \
    qml/diaryCardRecords.qml \
    qml/pageView.qml \
    qml/patternInput.qml \
    qml/recorder.qml \
    qml/testsEngine.qml \
    qml/testsResults.qml \
    qml/text.qml \
    res/TODO \
    Tablature.qml \
    PianoMap.qml \
    android/AndroidManifest.xml \
    android/build.gradle \
    android/gradle.properties \
    android/gradle/wrapper/gradle-wrapper.jar \
    android/gradle/wrapper/gradle-wrapper.properties \
    android/gradlew \
    android/gradlew.bat \
    android/res/values/libs.xml \
    libs/cqt/COPYING \
    libs/cqt/README \
    libs/fft/license.txt \
    libs/fft/readme.txt \
    libs/filters2/COPYING \
    libs/filters2/README.md \
    main.qml \
    qml/ACFQML.qml \
    qml/CQTQML.qml \
    qml/CepstrumQML.qml \
    qml/ConsoleLog.qml \
    qml/FiltersQML.qml \
    qml/STFTQML.qml \
    qml/WaveshapeQML.qml \
    qml/audioHandler.qml \
    qml/consoleLog.qml \
    qml/main.qml


ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android

