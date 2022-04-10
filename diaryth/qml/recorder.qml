import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Dialogs 1.1
import QtQuick.Controls 1.4 as Quick1

Item {
    id: recorderItem


    ColumnLayout
    {
        spacing: 10

        RowLayout
        {
            spacing: 10

            ToolButton {
                //property bool recording: false
                text: "Start" //recording ? "Start" : "Pause"
                onClicked: {
                    //recording = !recording
                    connector.started = false
                    recorder.start()
                }
            }

            ToolButton {
                text: "Stop and save"
                onClicked:  {
                    recorder.stop()
                    statusText.text = "Record was saved"
                    notificationAnimation.start()
                }
            }

            ToolButton {
                text: "Pause"
                onClicked:  {
                    recorder.pause()
                    statusText.text = "Record was paused"
                    notificationAnimation.start()
                }
            }

            ToolButton {
                text: "Cancel"
                onClicked: {
                    recorder.cancel()
                    statusText.text = "Record was canceled"
                    notificationAnimation.start()
                }
            }

            Text {
                id: dbsText
                text: "Volume lvl"
            }

            Text {
                id: timeText
                text: "Time ms"
            }

            Button {
                text: "Settings"
                onClicked:  {
                    settingsDialog.requestSettings()
                    settingsDialog.open()
                }
            }

            Button {
                text: "Back to calendar"
                onClicked: {
                    mainWindow.requestCalendar()
                }
            }
        }
        Text {
            opacity: 0.0
            id: statusText
            text: "Record was added"
            Layout.alignment: Qt.AlignCenter
        }
    }


    NumberAnimation
    {
        id: notificationAnimation
        target: statusText
        property: "opacity"
        duration: 1500
        from: 1.0
        to: 0.0
    }


    Dialog
    {
        id: settingsDialog

        width: 700
        height: 300

        x: 200
        y: 100

        ColumnLayout {
            RowLayout
            {
                Text{
                    text: "Device: "
                }
                ComboBox {
                    id: deviceCombo
                    implicitWidth: settingsDialog.width - 120
                }
            }
            RowLayout
            {
                Text{
                    text: "Codec: "
                }
                ComboBox {
                    id: codecCombo
                    implicitWidth: settingsDialog.width - 120
                }
            }
            RowLayout
            {
                Text{
                    text: "Container: "
                }
                ComboBox {
                    id: containerCombo
                    implicitWidth: settingsDialog.width - 120
                }
            }
            RowLayout
            {
                Text{
                    text: "Sample rate: "
                }
                ComboBox {
                    id: sampleRateCombo
                    implicitWidth: settingsDialog.width - 120
                }
            }
            Button {
                text: "Save"
                onClicked:  {
                    recorderItem.setInputDevice(deviceCombo.currentText)
                    recorderItem.setAudioCodec(codecCombo.currentText)
                    recorderItem.setFileContainer(containerCombo.currentText)
                    recorderItem.setSampleRate(sampleRateCombo.currentText)
                    settingsDialog.close()
                }
            }
            Button {
                text: "Close"
                onClicked: settingsDialog.close()
            }
        }

        function requestSettings() {
            deviceCombo.model = recorder.inputDevices()
            codecCombo.model = recorder.audioCodecs()
            containerCombo.model = recorder.fileContainers()
            sampleRateCombo.model = recorder.sampleRates()
        }
    }


    Connections {

        id: connector
        property bool started: false

        target: recorder

        function onTimeUpdate(ms) {
            timeText.text = ms
            if (connector.started === false) {
                connector.started = true
                console.log("Started recording!")
            }
        }

        function onDbsUpdate(dbs) {
            dbsText.text = dbs
        }
    }


    function keyboardEventSend(key, mode) {
        //Заглушка обработки клавиш
    }

}
