import QtQuick 2.12
import diaryth 1.0
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import QtQuick.Dialogs 1.1

Item
{
    id: item

    property string filename: "default"

    function reloadFile() {

    }

    ScrollView
    {
        id: scroll
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



            VisualReport
            {
                id: visualReport1
                height:  190
                width: 3000
                y: 5
                Component.onCompleted: {
                    visualReport1.setAmpitudeType()
                    flick.contentWidth = visualReport1.getFullWidth()
                    visualReport1.width = visualReport1.getFullWidth()
                }

                MouseArea
                {
                    anchors.fill: parent
                    onClicked:{

                    }
                    onDoubleClicked: {
                        var idx = visualReport1.eventIdxOnClick(mouseX, mouseY)
                        visualReport1.selectEvent(idx)
                        visualReport2.selectEvent(idx)
                    }

                }
            }

            VisualReport
            {
                id: visualReport2
                height:  500
                width: 3000
                y: 5 + visualReport1.height
                Component.onCompleted: {
                    visualReport2.setPitchType()
                    visualReport2.width =  visualReport2.getFullWidth()
                }

                MouseArea
                {
                    anchors.fill: parent
                    onClicked:{

                    }
                    onDoubleClicked: {
                        var idx = visualReport1.eventIdxOnClick(mouseX, mouseY)
                        visualReport1.selectEvent(idx)
                        visualReport2.selectEvent(idx)
                    }

                }
            }


        }
    }


    Popup
    {
        id: popup
        x: 50
        y: 50
        width: item.width - x * 2
        height: item.height - x * 2
        modal: true
        focus: true
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

        Repeater
        {
            id: eventsRepeater //TODO cover under flickable

            RowLayout
            {

                x: 10
                y: index * 40

                Text
                {
                    id: eventText
                }

                function setText(text)
                {
                    eventText.text = text
                }

                RoundButton {
                    text: "+"
                }
            }

        }

        Button
        {
            text: "Close"

            x: parent.width - width - 10
            y: parent.height - height - 5

            onClicked: popup.close()
        }
    }


    Button
    {
        text : "+"
        y: visualReport1.y + visualReport1.height + visualReport2.height

        onClicked:
        {
            var events = visualReport1.getSelectedEvents()

            eventsRepeater.model = 0
            eventsRepeater.model = events.length

            for (var i = 0; i < events.length; ++i)
            {
                var eventLine = events[i]
                eventsRepeater.itemAt(i).setText( eventLine[0] + " " + eventLine[1] + " " + eventLine[2] )
            }

            popup.open()
        }
    }


    function keyboardEventSend(key, mode) {
        //Заглушка, но можно реализовать логику здесь
    }

}
