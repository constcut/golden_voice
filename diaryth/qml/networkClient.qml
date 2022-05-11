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

        RowLayout {

            spacing: 10

            TextField {
                id: nameField
                placeholderText: "username"
            }
            TextField {
                id: tagsField
                placeholderText: "password"
            }
            Button
            {
                text: "Log In"
                onClicked:
                {

                }
            }
            Text {
                id: loginStatus
            }
        }

    }

    Connections
    {
        id: connector

        target: requestClient

        function onLoggedIn(value) {
            console.log("Logged in notification", value)
        }
    }



    function keyboardEventSend(key, mode) {
        //Заглушка обработки клавиш
    }

}
