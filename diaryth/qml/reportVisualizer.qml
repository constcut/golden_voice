import QtQuick 2.12
import diaryth 1.0
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import QtQuick.Dialogs 1.1

Item {
    id: item

    property string filename: "default"

    function reloadFile() {

    }

    ScrollView
    {
        width: parent.width
        height: visualReport1.height + visualReport2.height

        Flickable
        {
            id: flick
            y: 5
            x: 0
            width: parent.width
            height: visualReport1.height + visualReport2.height
            contentWidth: 3000
            contentHeight:  parent.height
            property int pressedX : 0

            MouseArea
            {
                x:0
                y:20
                width: parent.width
                height: 400

                onClicked:{


                }
                onDoubleClicked: {

                }

            }

            VisualReport
            {
                id: visualReport1
                height:  230
                width: 3000
                y: 5
                Component.onCompleted: {
                    visualReport1.setAmpitudeType()
                }
            }

            VisualReport
            {
                id: visualReport2
                height:  350
                width: 3000
                y: 5 + 250
                Component.onCompleted: {
                    visualReport2.setPitchType()
                }
            }


        }
    }

    function keyboardEventSend(key, mode) {
        //Заглушка, но можно реализовать логику здесь
    }

}
