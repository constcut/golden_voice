import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import QtQuick.Dialogs 1.1


Item {

    id: textItem

    property string date: ""
    property int localId: 0

    Connections
    {
        id: connector

        target: requestClient

        function onLoggedIn(value)
        {
            console.log("onLoggedIn", value)
            if (value)
            {
                loginStatus.text = "LOGGED IN"
                mainWindow.showButton()

                imageRow.visible = true
                reportRow.visible = true
                audioRow.visible = true
                view.visible = true
            }
        }

        function onFileSent(type, result)
        {
            if  (type === "audio")
                audioStatus.text = result

            if  (type === "image")
                imageStatus.text = result
        }

        function onFileProcessed(id, result)
        {
            processInfo.text = "DONE" //Возможно предлагать открыть визуализатор
            textArea.text = result
        }
    }



    ColumnLayout
    {
        x: 10
        y: 10

        spacing: 10

        RowLayout
        {

            spacing: 10

            TextField {
                id: username
                placeholderText: "username"

                text: "login"
            }
            TextField {
                id: password
                placeholderText: "password"

                text: "password"
            }
            Button
            {
                text: "Log In"
                onClicked:
                {
                    requestClient.logIn(username.text, password.text)
                }
            }
            Text {
                id: loginStatus
            }
        }

        RowLayout
        {
            id: audioRow
            visible: false

            TextField
            {
                id: filename
                placeholderText: "audio filename"
                text: "C:/audio.ogg"
                implicitWidth: 410
            }

            Button {
                text: "Send"
                onClicked: {
                    requestClient.sendAudioFile(filename.text)
                }
            }

            Text {
                id: audioStatus
            }

            spacing: 10

        }

        RowLayout
        {

            id: imageRow
            visible: false

            TextField
            {
                id: imagename
                placeholderText: "image filename"
                text: "C:/image.png"
                implicitWidth: 410
            }

            Button {
                text: "Send"
                onClicked: {
                    requestClient.sendImageFile(imagename.text)
                }
            }

            Text {
                id: imageStatus
            }

            spacing: 10

        }

        RowLayout
        {
            spacing: 10

            id: reportRow
            visible: false


            Button {
                text: "Check"
                onClicked: {
                    requestClient.requestCompleteStatus("1","2")
                }
            }

            Text {
                id: processInfo
            }
        }

        ScrollView
        {
            id: view
            width: 650
            clip: true

            visible: false

            implicitWidth: 650

            height: 300
            implicitHeight: 300

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
