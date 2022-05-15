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

                /*RoundButton {
                    text: "+"
                }*/
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


    RowLayout
    {
            y: visualReport1.y + visualReport1.height + visualReport2.height

            Button {
                text: "Remove selection"
                onClicked: {
                    visualReport1.removeAllSelections()
                    visualReport2.removeAllSelections()
                }
            }

            ComboBox {
                id: chunkId
            }

            Button {
                text: "Select chunk"
                onClicked: {
                    visualReport1.selectChunk(parseInt(chunkId.currentText))
                    visualReport2.selectChunk(parseInt(chunkId.currentText))
                }
            }


            Button
            {
                text : "+"

                onClicked:
                {
                    var events = visualReport1.getSelectedEvents()

                    eventsRepeater.model = 0
                    eventsRepeater.model = events.length + 4 //names, chunk, full, full - chunk

                    var fieldsNames = visualReport1.getPraatFieldsNames();
                    var fullText = "type, "

                    for (var j = 0; j < fieldsNames.length; ++j)
                        fullText += fieldsNames[j] + ", "

                    eventsRepeater.itemAt(0).setText( fullText )

                    for (var i = 0; i < events.length; ++i)
                    {
                        var eventLine = events[i]
                        var eventIdx = eventLine[0]
                        var word = visualReport1.getWordByIdx(eventIdx)

                        fullText = word + " "

                        for (j = 1; j < eventLine.length; ++j)
                            fullText += eventLine[j].toString() + ", "

                        eventsRepeater.itemAt(i + 1).setText( fullText )

                        //TODO полная разница full и chunk с каждым словом
                        //TODO графики полной разницы каждого слова
                    }

                    var chunkInfo = visualReport1.getChunkInfo(parseInt(chunkId.currentText))

                    fullText = "chunk "
                    for (j = 0; j < chunkInfo.length; ++j)
                        fullText += chunkInfo[j].toString() + ", "

                    eventsRepeater.itemAt( events.length + 1 ).setText( fullText )

                    var fullInfo = visualReport1.getFullInfo()

                    fullText = "full "
                    for (j = 0; j < fullInfo.length; ++j)
                        fullText += fullInfo[j].toString() + ", "

                    eventsRepeater.itemAt( events.length + 2 ).setText( fullText )

                    fullText = "diff "
                    for (j = 0; j < chunkInfo.length; ++j) {
                        fullText += (fullInfo[j] - chunkInfo[j]).toFixed(3).toString() + ", "
                    }

                    eventsRepeater.itemAt( events.length + 3 ).setText( fullText )

                    popup.open()
                }
            }
    }

    JsonReport
    {
        id: jsonReport

        Component.onCompleted:
        {
            flick.contentWidth = jsonReport.getFullWidth()
            visualReport1.width = jsonReport.getFullWidth()
            visualReport2.width =  jsonReport.getFullWidth()
            chunkId.model = jsonReport.getChunksCount()

            visualReport1.setParent(jsonReport)
            visualReport2.setParent(jsonReport)

            visualReport1.addPraatField("Jitter (rap)", "red", 20)
            visualReport1.addPraatField("Number of pulses", "orange", 1.2)
            visualReport1.setPraatType()

            visualReport2.setPitchType()

        }

    }




    function keyboardEventSend(key, mode) {
        //Заглушка, но можно реализовать логику здесь
    }

}
