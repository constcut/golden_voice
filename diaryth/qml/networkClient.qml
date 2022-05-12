import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import QtQuick.Dialogs 1.1


Item {

    id: textItem

    property string date: ""
    property int localId: 0


    Component.onCompleted:
    {

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

                text: "testlogin"
            }
            TextField {
                id: password
                placeholderText: "password"

                text: "testpassword"
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

            TextField
            {
                id: filename
                placeholderText: "audio filename"
                text: "C:/Users/constcut/Desktop/local/local_2.ogg"
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

            TextField
            {
                id: imagename
                placeholderText: "image filename"
                text: "C:/Users/constcut/Desktop/local/sprint.png"
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

    }

    Connections
    {
        id: connector

        target: requestClient

        function onLoggedIn(value)
        {
            if (value)
                loginStatus.text = "You are logged in!"
            else
                loginStatus.text = "Username or password incorrect"
        }

        function onFileSent(type, result)
        {
            if  (type === "audio")
                audioStatus.text = result

            if  (type === "image")
                imageStatus.text = result
        }
    }



    function keyboardEventSend(key, mode) {
        //Заглушка обработки клавиш
    }

}
