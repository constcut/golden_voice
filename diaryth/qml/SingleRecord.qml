import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Dialogs 1.1

import QtQuick.Controls 1.4 as Quick1

Item {
    id: singleRecordItem

    property string date: ""
    property int localId: 0

    ColumnLayout
    {
        x: 40
        y: 40

        spacing: 10

        TextField {
            id: nameField
        }
        Text {
            id: dateTimeText
        }
        Text {
            id: durationMsText
        }

        TextField {
            id: tagsField
            placeholderText: "tags"
        }
        TextField {
            id: descriptionField
            placeholderText: "description"
        }
        Button {
            text: "Save"
            onClicked: {
                sqlBase.editAudioRecord(singleRecordItem.date, singleRecordItem.localId,
                                        nameField.text, tagsField.text, descriptionField.text)
            }
        }
        Button {
            text: "Play"
            onClicked: {
                recorder.playFile(singleRecordItem.date, singleRecordItem.localId)
            }
        }

        Button {
            text: "Back to calendar"
            onClicked: {
                mainWindow.requestCalendar()
            }
        }
    }


    Component.onCompleted:
    {
        var record =
            sqlBase.findSingleRecord(singleRecordItem.date,
                                     singleRecordItem.localId)

        dateTimeText.text = record[0] + " T " + record[1]
        nameField.text = record[3]
        durationMsText.text = record[4]

        tagsField.text = record[5]
        descriptionField.text = record[6]
    }


    function keyboardEventSend(key, mode) {
        //Заглушка обработки клавиш
    }

}
