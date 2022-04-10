import QtQuick 2.15
import QtQuick.Window 2.15

import QtQml 2.15

import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

ApplicationWindow {

    id: mainWindow
    visible: true
    title: "diaryth"

    x: 30
    y: 50

    Component.onCompleted: {
        if (Qt.platform.os === "android") {
            exitMenuItem.visible = false
            exitMenuItem.height = 0
            mainWindow.visibility = "FullScreen"
        }
        else {
            mainWindow.width = 1280
            mainWindow.height = 600
        }

        mainLoader.setSource("diaryCard.qml") //calendar, diaryCard,
    }


    onClosing: {
        if (Qt.platform.os == "android") {
            close.accepted = false
            mainMenu.open()
        }
        else
            close.accepted = true
    }


    ToolButton {
        id: mainMenuButton
        text: ":"
        x: parent.width - width - 5
        y: 5
        onClicked: {
            mainMenu.x = parent.width - mainMenu.width
            mainMenu.open()
        }
    }


    Menu {
        id: mainMenu
        MenuItem {
            text: "Console"
            onTriggered: {
                mainLoader.setSource("consoleLog.qml")
                mainLoader.focus = true
            }
        }
        MenuItem {
            text: "Config"
            onTriggered: {
                mainLoader.setSource("config.qml")
            }
        }
        MenuItem {
            text: "Page"
            onTriggered: mainLoader.setSource("pageView.qml")
        }
        MenuItem {
            text: "Audio"
            onTriggered: mainLoader.setSource("audioHandler.qml")
        }
        MenuItem {
            text: "Recorder"
            onTriggered: mainLoader.setSource("recorder.qml")
        }
        MenuItem {
            text: "Text"
            onTriggered: mainLoader.setSource("text.qml")
        }
        MenuItem {
            text: "Diary card"
            onTriggered: mainLoader.setSource("diaryCard.qml")
        }
        MenuItem {
            text: "Calendar"
            onTriggered: mainLoader.setSource("calendar.qml")
        }
        MenuItem {
            text: "Tests"
            onTriggered: mainLoader.setSource("testsEngine.qml")
        }


        MenuItem {
            id: exitMenuItem
            text: "Exit"
            onTriggered:  {
                if (Qt.platform.os !== "android")
                    Qt.exit(0)
            }

        }
    }


    function requestWaveshape(filename) {
        //Delayed: open many tabs, just like in athenum
        mainLoader.setSource("qrc:/qml/WaveshapeQML.qml",
                             {"filename": filename});
        mainLoader.focus = true
    }

    function requestAFC(filename) {
        mainLoader.setSource("qrc:/qml/ACFQML.qml",
                             {"filename": filename});
        mainLoader.focus = true
    }

    function requestSTFT(filename) {
        mainLoader.setSource("qrc:/qml/STFTQML.qml",
                             {"filename": filename});
        mainLoader.focus = true
    }

    function requestCQT(filename) {
        mainLoader.setSource("qrc:/qml/CQTQML.qml",
                             {"filename": filename});
        mainLoader.focus = true
    }

    function requestFilters(filename) {
        mainLoader.setSource("qrc:/qml/FiltersQML.qml",
                             {"filename": filename});
        mainLoader.focus = true
    }

    function requestCepstrum(filename) {
        mainLoader.setSource("qrc:/qml/CepstrumQML.qml",
                             {"filename": filename});
        mainLoader.focus = true
    }

    function requestSingleRecord(date, localId) {
        mainLoader.setSource("qrc:/qml/SingleRecord.qml",
                             {"date": date, "localId":localId})
        mainLoader.focus = true
    }

    function requestSingleText(date, localId) {
        mainLoader.setSource("qrc:/qml/text.qml",
                             {"date": date, "localId":localId})
        mainLoader.focus = true
    }

    function requestAddAudioRecord() {
        mainLoader.setSource("recorder.qml")
        mainLoader.focus = true
    }

    function requestAddText() {
        mainLoader.setSource("text.qml")
        mainLoader.focus = true
    }

    function requestCalendar() {
        mainLoader.setSource("calendar.qml")
        mainLoader.focus = true
    }

    function requestTestsResults() {
        mainLoader.setSource("testsResults.qml")
        mainLoader.focus = true
    }

    function requestTestsEngine() {
        mainLoader.setSource("testsEngine.qml")
        mainLoader.focus = true
    }

    function requestCardRecords() {
        mainLoader.setSource("diaryCardRecords.qml")
        mainLoader.focus = true
    }

    function requestDiaryCard() {
        mainLoader.setSource("diaryCard.qml")
        mainLoader.focus = true
    }

    //Реализовать более удобную навигацию, используя свайп
    //https://doc.qt.io/qt-5/qml-qtquick-controls2-swipeview.html
    //Возможно даже по команде загружать несколько свайпов, вместо одного Loader'a N штук


    Loader
    {
        id:mainLoader
        anchors.fill: parent
        focus: true
        Keys.onPressed:
        {
            if (event.key === Qt.Key_Home)
                mainMenu.open()

            mainLoader.item.keyboardEventSend(event.key, event.modifiers)
        }
    }

}
