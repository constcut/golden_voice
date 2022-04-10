import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Dialogs 1.1
import QtQuick.Controls 1.4 as Quick1

Item {

    id: textItem

    property string date: ""
    property int localId: 0


    Component.onCompleted:
    {
        if (textItem.date.length != 0)
        {
            var singleText = sqlBase.findSingleText(textItem.date, textItem.localId)
            nameField.text = singleText[3]
            textArea.text = singleText[4]
            tagsField.text = singleText[5]
            descriptionField.text = singleText[6]
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


    ColumnLayout
    {
        x: 10
        y: 10

        spacing: 10

        Text {
            opacity: 0.0
            id: statusText
            text: "Record was added"
            Layout.alignment: Qt.AlignCenter
        }

        RowLayout {

            spacing: 10

            TextField {
                id: nameField
                placeholderText: "Name"
            }
            TextField {
                id: tagsField
                placeholderText: "tags"
            }
            TextField {
                id: descriptionField
                placeholderText: "description"
            }
            Button
            {
                text: "Save text"
                onClicked:
                {
                    if (textItem.date.length != 0)
                    {
                        sqlBase.editText(textItem.date, textItem.localId,
                                         nameField.text, tagsField.text,
                                         descriptionField.text)
                    }
                    else
                    {
                        sqlBase.addText(nameField.text, textArea.text, tagsField.text,
                                        descriptionField.text)

                        nameField.text = "" //Защита от повторного добавления
                        tagsField.text = ""
                        descriptionField.text = ""
                        textArea.text = ""
                        //Возможно стоит переключать на другой экран
                    }

                    notificationAnimation.start()
                }
            }

            Button {
                text: "Back to calendar"
                onClicked: {
                    mainWindow.requestCalendar()
                }
            }
        }

        Popup {
            id: popup
            x: 100
            y: 100
            width: 200
            height: 300
            modal: true
            focus: true
            closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutsideParent
        }

        ScrollView {
            id: view
            width: 650
            clip: true

            implicitWidth: 650

            TextArea {

                background: Rectangle {
                    //color: "lightgreen"
                    border.color: "lightgreen"
                }

                implicitWidth: parent.width

                id: textArea
                text: ""
                placeholderText: "Input text here"

                readOnly: textItem.date.length != 0
            }

            ScrollBar.vertical.policy: ScrollBar.AlwaysOn
        }
    }



    function keyboardEventSend(key, mode) {
        //Заглушка обработки клавиш
    }

}
