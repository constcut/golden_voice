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
                        var idx = jsonReport.eventIdxOnClick(mouseX, mouseY)
                        jsonReport.selectEvent(idx)
                    }

                }
            } //VisualReport

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
                        var idx = jsonReport.eventIdxOnClick(mouseX, mouseY)
                        jsonReport.selectEvent(idx)
                    }

                }
            } //VisualReport

        } //Flickable

    } //ScrollView


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

                Text {
                    id: eventText
                }

                function setText(text) {
                    eventText.text = text
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


    RowLayout
    {
            y: visualReport1.y + visualReport1.height + visualReport2.height

            Button {
                text: "Remove selection"
                onClicked: {
                    jsonReport.removeAllSelections()
                }
            }

            ComboBox {
                id: chunkId
            }

            Button {
                text: "Select chunk"
                onClicked: {
                    jsonReport.selectChunk(parseInt(chunkId.currentText))
                }
            }


            Button
            {
                text : "+"

                onClicked:
                {
                    var events = jsonReport.getSelectedEvents()

                    eventsRepeater.model = 0
                    eventsRepeater.model = events.length + 4 //names, chunk, full, full - chunk

                    var fieldsNames = jsonReport.getPraatFieldsNames();
                    var fullText = "type, "

                    for (var j = 0; j < fieldsNames.length; ++j)
                        fullText += fieldsNames[j] + ", "

                    eventsRepeater.itemAt(0).setText( fullText )

                    for (var i = 0; i < events.length; ++i)
                    {
                        var eventLine = events[i]
                        var eventIdx = eventLine[0]
                        var word = jsonReport.getWordByIdx(eventIdx)

                        fullText = word + " "

                        for (j = 1; j < eventLine.length; ++j)
                            fullText += eventLine[j].toString() + ", "

                        eventsRepeater.itemAt(i + 1).setText( fullText )

                        //TODO полная разница full и chunk с каждым словом
                        //TODO графики полной разницы каждого слова
                    }

                    var chunkInfo = jsonReport.getChunkInfo(parseInt(chunkId.currentText))

                    fullText = "chunk "
                    for (j = 0; j < chunkInfo.length; ++j)
                        fullText += chunkInfo[j].toString() + ", "

                    eventsRepeater.itemAt( events.length + 1 ).setText( fullText )

                    var fullInfo = jsonReport.getFullInfo()

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
            } //Button

            Button {
                text: "Config"

                onClicked: {
                    configPopup.loadFromVisual(visualReport1)
                    configPopup.open()
                }
            }
    }


    Popup
    {
        id: configPopup
        x: 50
        y: 50
        width: item.width - x * 2
        height: item.height - x * 2
        modal: true
        focus: true
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

        DoubleValidator {
            id: realValidator
        }

        property var colorsNames: ["red", "green", "blue", "orange", "black", "darkRed", "darkGreen", "darkBlue",
                                   "cyan", "magenta", "yellow", "gray", "darkCyan", "darkMagenta",
                                   "darkYellow", "darkGray", "lightGray"]

        property var lastVisualReport: ""

        function loadFromVisual(visualReport)
        {
            configPopup.lastVisualReport = visualReport

            var praatFields = visualReport.getPraatFields()

            configRepeater.model = 0
            configRepeater.model = praatFields.length

            for (var i = 0; i < praatFields.length; ++i)
            {
                var fieldLine = praatFields[i]
                configRepeater.itemAt(i).loadValues(fieldLine[0], fieldLine[1], fieldLine[2])
            }
        }


        Repeater
        {
            model: 3
            id: configRepeater //TODO cover under flickable

            RowLayout
            {
                y: index * 60

                function loadValues(name, color, coef)
                {
                    console.log("LOAD ", name, color, coef)

                    var praatFieldsNames = jsonReport.getPraatFieldsNames()

                    for (var i = 0; i < praatFieldsNames.length; ++i)
                    {
                        if (praatFieldsNames[i] === name)
                            break;
                    }

                    praatFieldName.currentIndex = i

                    for (i = 0; i < configPopup.colorsNames.length; ++i)
                    {
                        if (configPopup.colorsNames[i] === color)
                            break;
                    }

                    fieldColor.currentIndex = i

                    fieldCoef.text = coef
                }

                function getName() {
                    return praatFieldName.currentText
                }

                function getColor() {
                    return fieldColor.currentText
                }

                function getYCoef() {
                    return fieldCoef.text
                }

                ComboBox
                {
                    id: praatFieldName
                    model: jsonReport.getPraatFieldsNames()

                    implicitWidth: 250
                    width: 250
                }

                ComboBox {
                    id: fieldColor
                    model: configPopup.colorsNames
                }

                TextField {
                    id: fieldCoef
                    validator: realValidator
                    text: "1,0"
                }
            }
        }

        Button
        {
            text: "Close"

            x: parent.width - width - 10
            y: parent.height - height - 5

            onClicked: configPopup.close()
        }

        Button
        {
            text: "Save"

            x: 10
            y: parent.height - height - 5

            onClicked:
            {
                configPopup.lastVisualReport.clearPraatFields() //find a way to use parametric values

                for (var i = 0; i < configRepeater.model; ++i)
                {
                    console.log("Fillin field #", i)

                    var name = configRepeater.itemAt(i).getName()
                    var color = configRepeater.itemAt(i).getColor()
                    var yCoef = parseFloat(configRepeater.itemAt(i).getYCoef())

                    configPopup.lastVisualReport.addPraatField(name, color, yCoef)
                }

                configPopup.lastVisualReport.setPraatType() //way to repaint

                configPopup.close()
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
