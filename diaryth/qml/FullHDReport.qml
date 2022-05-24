import QtQuick 2.12
import diaryth 1.0
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import QtQuick.Dialogs 1.1


Item
{
    id: fullHDReport

    FileDialog
    {
        id: fileDialog
        title: "Please choose json speech report"
        folder: shortcuts.desktop
        onAccepted: {
            var filename = fileDialog.fileUrls[0].substring(8)
            jsonReport.loadFromFile(filename)
            chunkId.model = jsonReport.getChunksCount()
            reloadVisualReports()
            fileDialog.close()
        }
        onRejected: {
            fileDialog.close()
        }
        nameFilters: [ "JSON report (*.json)" ]
    }


    function requestFileDialog()
    {
        fileDialog.open()
        reloadVisualReports()
    }

    JsonReport
    {
        id: jsonReport
    }


    function reloadVisualReports()
    {
        flick.contentWidth = jsonReport.getFullWidth()
        for (var i = 0; i < reportsRepeater.model; ++i)
        {
            reportsRepeater.itemAt(i).setParent(jsonReport)
            reportsRepeater.itemAt(i).width = jsonReport.getFullWidth()
            reportsRepeater.itemAt(i).setType(fullHDReport.reportsTypes[i])
        }

        scroll.updatePositions()
    }


    property var reportsHeight: [ 220, 35, 220, 500, 250, 220 ]
    property var reportsTypes: [VisualTypes.PraatInfo, VisualTypes.PlainWords, VisualTypes.ReportFields, VisualTypes.Pitch, VisualTypes.Amplitude, VisualTypes.ChunksOnly]


    function calculateY(idx)
    {
        var hSum = 0
        for (var i = 1; i <= idx; ++i)
            hSum += reportsHeight[i - 1]

        var yValue =  hSum + 5 * (idx + 1)
        return yValue
    }

    RowLayout
    {
        x: 25
        y: 10

        Text {
            text: "Y coef: "
        }

        TextField
        {
            id: infoCoef
            text: "1"
            width: 50
            implicitWidth: 50

            placeholderText: "Y Coef"

            validator: realValidator
        }

        Text {
            text: "Zoom"
        }

        Button
        {
            text: "Save config"

            onClicked: jsonReport.saveLocalConfig()
        }

        Button
        {
            text: "Load config"

            onClicked:
            {
                var heights = jsonReport.loadLocalConfig()

                for (var i = 0; i < heights.length; ++i)
                {
                    fullHDReport.reportsHeight[i] = heights[i]
                    //reportsRepeater.itemAt(i).height = heights[i]
                    fullHDReport.reportsTypes[i] = reportsRepeater.itemAt(i).getType()
                }

                scroll.updatePositions()

                for (i = 0; i < heights.length; ++i)
                     reportsRepeater.itemAt(i).setType(fullHDReport.reportsTypes[i])
            }
        }

        Button
        {
            text: "Open report file"

            onClicked: {
                fileDialog.open()
            }
        }

        RoundButton
        {
            text: "Zoom+"
            onClicked:
            {
                jsonReport.setZoom(jsonReport.getZoom() * 2)
                flick.contentWidth = jsonReport.getFullWidth()

                for (var i = 0; i < reportsRepeater.model; ++i)
                    reportsRepeater.itemAt(i).width = jsonReport.getFullWidth()
            }
        }

        RoundButton
        {
            text: "Zoom-"
            onClicked: {
                jsonReport.setZoom(jsonReport.getZoom() / 2)
                flick.contentWidth = jsonReport.getFullWidth()

                for (var i = 0; i < reportsRepeater.model; ++i)
                    reportsRepeater.itemAt(i).width = jsonReport.getFullWidth()
            }
        }

        Button
        {
            text: "Unselect"
            onClicked: {
                jsonReport.removeAllSelections()
            }

            width: 80
            implicitWidth: 80
        }

        ComboBox
        {
            id: chunkId

            width: 40
            implicitWidth: 40

            ToolTip.visible: hovered
            ToolTip.text: "Chunk id"
        }

        Button {
            text: "Select chunk"
            onClicked: {
                jsonReport.selectChunk(parseInt(chunkId.currentText))
            }
        }


        Button
        {
            text: "Data markup"

            onClicked:
            {
                var tagsAndComments = jsonReport.getSelectedEventsMarkup();
                markupPopup.loadMarkup(tagsAndComments[0], tagsAndComments[1],
                                       jsonReport.getSelectedEventsString())
                markupPopup.open()
            }
        }

        Button
        {
            text: "All words"
        }

        Button
        {
            text: "All tags"
        }
    }


    Drawer {

    }



    ScrollView
    {
        width: parent.width
        height: parent.height - 300//fullHDReport.reportsHeight[0] + reportsHeight[1] + reportsHeight[2] + reportsHeight[3] + reportsHeight[4] //TODO function
        clip: true

        y: 50

        Flickable
        {

            id: hFlick
            y: 5
            x: 0
            width: parent.width
            height: parent.height
            clip: true

            contentWidth: parent.width
            contentHeight: fullHDReport.reportsHeight[0] + reportsHeight[1] + reportsHeight[2] + reportsHeight[3] + reportsHeight[4] + reportsHeight[5] + 60  ///TODO


            ScrollBar.vertical: ScrollBar
            {
                id: hScroll
                width: 40
                policy: ScrollBar.AlwaysOn
            }


            ScrollView
            {
                y: 0

                id: scroll
                width: parent.width
                height: parent.height ///TODO



                function updatePositions()
                {
                    for (var i = 0; i < reportsRepeater.model; ++i)
                    {
                        var newY = fullHDReport.calculateY(i)
                        reportsRepeater.itemAt(i).y = fullHDReport.calculateY(i)
                        reportsRepeater.itemAt(i).height = fullHDReport.reportsHeight[i]
                    }

                }


                Flickable
                {
                    id: flick
                    y: 5
                    x: 0
                    width: parent.width
                    height: scroll.height
                    contentWidth: 3000
                    contentHeight:  parent.height
                    property int pressedX : 0


                    ScrollBar.horizontal: ScrollBar
                    {

                        height: 30
                        policy: ScrollBar.AlwaysOn

                    }

                    onContentXChanged:
                    {

                        if (contentX > (contentWidth - width - 10))
                        {
                            contentX = 0
                            console.log("Scrollback")
                        }
                    }


                    Repeater
                    {
                        id: reportsRepeater
                        model: 6

                        VisualReport
                        {
                            id: visualReport

                            height: fullHDReport.reportsHeight[index]
                            width: 3000
                            y: fullHDReport.calculateY(index)

                            ToolTip {
                                id: localToolTip
                            }

                            /*Component.onDestroyed:
                            {
                                jsonReport.removeVisual(visualReport)
                            }*/ //We need it if reportsRepeater.model will change

                            MouseArea
                            {
                                anchors.fill: parent
                                acceptedButtons: Qt.LeftButton | Qt.RightButton

                                onClicked:
                                {
                                    if (mouse.button === Qt.LeftButton)
                                    {
                                        var realY = (visualReport.height - mouseY) / parseFloat(infoCoef.text)
                                        var seconds = mouseX / jsonReport.getZoom()
                                        localToolTip.show("value= " + realY + " time= " + seconds + "s")
                                        localToolTip.x = mouseX + 2
                                        localToolTip.y = mouseY + 2
                                    }
                                }

                                onWheel:
                                {
                                    if (wheel.angleDelta.y > 0 && hScroll.position >= 0.05)
                                        hScroll.position -= 0.05
                                    else if (hScroll.position <= 0.45)
                                        hScroll.position += 0.05

                                    //hScroll.position += 10
                                    print(hScroll.position)
                                }

                                onDoubleClicked:
                                {
                                    if (mouse.button == Qt.RightButton)
                                    {
                                        var reportType = visualReport.getType()

                                        if (reportType === VisualTypes.PraatInfo || reportType === VisualTypes.PraatInfoFullDiff ||
                                            reportType === VisualTypes.PraatInfoChunkDiff || reportType === VisualTypes.ChunksOnly)
                                        {
                                            praatFieldsConfigPopup.loadFromVisual(visualReport)
                                            praatFieldsConfigPopup.open()
                                        }

                                        if (reportType === VisualTypes.ReportFields)
                                        {
                                            reportFieldsConfigPopup.loadFromVisual(visualReport)
                                            reportFieldsConfigPopup.open()
                                        }
                                    }
                                    else
                                    {
                                        var idx = jsonReport.eventIdxOnClick(mouseX, mouseY)
                                        jsonReport.selectEvent(idx)
                                    }
                                }

                                onPressAndHold:
                                {
                                    if (mouse.button == Qt.RightButton)
                                    {
                                        configVisualReportPopup.connectWithReport(visualReport, index)
                                        configVisualReportPopup.open()
                                    }
                                }

                            }
                        }


                    }

                } //Flickable

            } //ScrollView

        }
    }




//=====================Data markup=========================================================

    Popup
    {
        id: markupPopup

        x: fullHDReport.width/2 - width/2 - 25
        y: fullHDReport.height - height - 50

        width: 900
        height: 450

        modal: true
        focus: true
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside


        function loadMarkup(tags, comments, selectedString)
        {
            tagsArea.text = tags
            commentsArea.text = comments
            datasetInfo.text = selectedString
        }

        ColumnLayout
        {
            spacing: 10

            Text
            {
                id: datasetInfo
                text: ""
            }

            Item
            {
                height: tagsArea.height + 15

                Text {
                    text: "Tags: "
                }

                ScrollView
                {
                    x: 150

                    width: 650
                    clip: true
                    implicitWidth: 650
                    implicitHeight: 150

                    TextArea {

                        background: Rectangle {
                            border.color: "lightgreen"
                        }

                        implicitWidth: parent.width

                        id: tagsArea
                        text: ""
                        placeholderText: "Input tags here"
                    }

                    ScrollBar.vertical.policy: ScrollBar.AlwaysOn
                }
            }

            Item
            {
                //spacing: 10

                height: commentsArea.height + 15

                Text {
                    text: "Comments: "
                }

                ScrollView
                {
                    x: 150

                    width: 650
                    clip: true
                    implicitWidth: 650
                    implicitHeight: 150

                    TextArea {

                        background: Rectangle {
                            border.color: "lightgreen"
                        }

                        implicitWidth: parent.width

                        id: commentsArea
                        text: ""
                        placeholderText: "Input comments here"
                    }

                    ScrollBar.vertical.policy: ScrollBar.AlwaysOn
                }
            }

            Item
            {
                height: 60

                Button
                {
                    x: 15
                    text: "Cancel"

                    onClicked: {
                        markupPopup.close()
                    }
                }

                Button
                {
                    x: markupPopup.width - width - 35
                    text: "Save"

                    onClicked:
                    {
                        jsonReport.saveSelectedEventsMarkup(tagsArea.text, commentsArea.text)
                        markupPopup.close()
                    }
                }
            }
        }

    }




//================Configure single report view=============================================

    Popup
    {
        id: configVisualReportPopup

        x: fullHDReport.width/2 - width/2 - 25
        y: fullHDReport.height - height - 50

        width: 500
        height: 160

        modal: true
        focus: true
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

        property var lastVisualReport: "" //undefined?
        property int reportIndex: 0

        function connectWithReport(visualReport, idx)
        {
            configVisualReportPopup.lastVisualReport = visualReport
            configVisualReportPopup.reportIndex = idx

            var reportType = visualReport.getType()
            visualReportType.currentIndex = reportType - 1
            reportHeightSlider.value = visualReport.height

            for (var i = 0; i < reportsRepeater.model; ++i)
                reportsRepeater.itemAt(i).setShowBorder(true)
        }

        onClosed:
        {
            for (var i = 0; i < reportsRepeater.model; ++i)
                reportsRepeater.itemAt(i).setShowBorder(false)
        }


        ComboBox
        {
            y: 20
            x: 25
            width: parent.width - 50

            id: visualReportType
            model: ["Pitch", "Intensity", "Praat", "PraatChunkDiff", "PraatFullDiff",
                    "Chunks", "Words", "ReportFields"]

            onCurrentTextChanged:
            {
                if (configVisualReportPopup.lastVisualReport !== "")
                {
                    configVisualReportPopup.lastVisualReport.setType(currentIndex + 1)
                    //jsonReport.saveLocalConfig()
                }
            }
        }

        Slider
        {
            id: reportHeightSlider

            y: 30 + visualReportType.height

            x: 25
            width: parent.width - 50

            from: 35
            to: 600
            stepSize: 5

            value: 190

            onMoved:
            {
                fullHDReport.reportsHeight[configVisualReportPopup.reportIndex] = value
                configVisualReportPopup.lastVisualReport.height = value
                scroll.updatePositions()
                //jsonReport.saveLocalConfig()
            }
        }

    }



//================Configure single report view=============================================



//=====Visual reports fields popups========================================================
//=========================================================================================


    Popup
    {
        id: praatFieldsConfigPopup
        x: 50
        y: 50
        width: fullHDReport.width - x * 2
        height: fullHDReport.height - x * 2
        modal: true
        focus: true
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

        DoubleValidator {
            id: realValidator
        }

        property var colorsNames: ["red", "green", "blue", "orange", "black", "darkRed", "darkGreen", "darkBlue",
                                   "cyan", "magenta", "yellow", "gray", "darkCyan", "darkMagenta",
                                   "darkYellow", "darkGray", "lightGray"]

        property var lastVisualReport: []
        property var storedFields: []

        function loadFromVisual(visualReport)
        {
            praatFieldsConfigPopup.lastVisualReport = visualReport

            var praatFields = visualReport.getPraatFields()

            configRepeater.model = 0
            configRepeater.model = praatFields.length

            for (var i = 0; i < praatFields.length; ++i)
            {
                var fieldLine = praatFields[i]
                configRepeater.itemAt(i).loadValues(fieldLine[0], fieldLine[1], fieldLine[2])
            }
        }


        function storeFields(skipIdx = -1)
        {
            var fieldsList = []

            for (var i = 0; i < configRepeater.model; ++i)
            {
                if (i === skipIdx)
                    continue

                var name = configRepeater.itemAt(i).getName()
                var color = configRepeater.itemAt(i).getColor()
                var yCoef = configRepeater.itemAt(i).getYCoef()
                var fieldLine = [name, color, yCoef]

                fieldsList.push(fieldLine)
            }

            praatFieldsConfigPopup.storedFields = fieldsList
        }

        function loadFields()
        {
            for (var i = 0; i < praatFieldsConfigPopup.storedFields.length; ++i)
            {
                var fieldLine = praatFieldsConfigPopup.storedFields[i]
                configRepeater.itemAt(i).loadValues(fieldLine[0], fieldLine[1], fieldLine[2])
            }
        }



        Repeater
        {
            model: 3
            id: configRepeater

            RowLayout
            {
                y: index * 60

                function loadValues(name, color, coef)
                {
                    var praatFieldsNames = jsonReport.getPraatFieldsNames()

                    for (var i = 0; i < praatFieldsNames.length; ++i)
                    {
                        if (praatFieldsNames[i] === name)
                            break;
                    }

                    praatFieldName.currentIndex = i

                    for (i = 0; i < praatFieldsConfigPopup.colorsNames.length; ++i)
                    {
                        if (praatFieldsConfigPopup.colorsNames[i] === color)
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

                    implicitWidth: 400
                }

                ComboBox {
                    id: fieldColor
                    model: praatFieldsConfigPopup.colorsNames
                }

                TextField {
                    id: fieldCoef
                    validator: realValidator
                    text: "1,0"
                }

                RoundButton {
                    text: "-"

                    onClicked: {
                        praatFieldsConfigPopup.storeFields(index)
                        loadStoredTimer.start() //Hot fix for ReferenceError: xyz is not defined
                        configRepeater.model = configRepeater.model - 1 //WE got issue here as our current object is child
                    }
                }

                RoundButton {
                    text: "+"

                    onClicked:
                    {
                        praatFieldsConfigPopup.storeFields()
                        loadStoredTimer.start() //Hot fix for ReferenceError: xyz is not defined
                        configRepeater.model = configRepeater.model + 1 //WE got issue here as our current object is child
                    }
                }
            }
        }

        Timer
        {
            id: loadStoredTimer
            interval: 50
            running: false
            repeat: false
            onTriggered: {
                praatFieldsConfigPopup.loadFields()
            }
        }

        Button
        {
            text: "Add new"

            x: parent.width/2 - width/2 - 5
            y: parent.height - height - 5

            onClicked:
            {
                praatFieldsConfigPopup.storeFields()
                loadStoredTimer.start()
                configRepeater.model += 1 //решением этого хотфикса выше можно сделать за счёт запуска таймера с инкрементом и декриментом, но это не критично
            }
        }


        Button
        {
            text: "Close"

            x: parent.width - width - 10
            y: parent.height - height - 5

            onClicked: praatFieldsConfigPopup.close()
        }

        Button
        {
            text: "Save"

            x: 10
            y: parent.height - height - 5

            onClicked:
            {
                praatFieldsConfigPopup.lastVisualReport.clearPraatFields() //find a way to use parametric values

                for (var i = 0; i < configRepeater.model; ++i)
                {

                    var name = configRepeater.itemAt(i).getName()
                    var color = configRepeater.itemAt(i).getColor()
                    var yCoef = parseFloat(configRepeater.itemAt(i).getYCoef())

                    praatFieldsConfigPopup.lastVisualReport.addPraatField(name, color, yCoef)
                }

                var type = praatFieldsConfigPopup.lastVisualReport.getType()
                praatFieldsConfigPopup.lastVisualReport.setType(type) //trick for reloading

                praatFieldsConfigPopup.close()
            }
        }
    }




    Popup
    {
        id: reportFieldsConfigPopup
        x: 50
        y: 50
        width: fullHDReport.width - x * 2
        height: fullHDReport.height - x * 2
        modal: true
        focus: true
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

        property var colorsNames: ["red", "green", "blue", "orange", "black", "darkRed", "darkGreen", "darkBlue",
                                   "cyan", "magenta", "yellow", "gray", "darkCyan", "darkMagenta",
                                   "darkYellow", "darkGray", "lightGray"]

        property var lastVisualReport: []
        property var storedFields: []

        function loadFromVisual(visualReport)
        {
            reportFieldsConfigPopup.lastVisualReport = visualReport
            configFieldsRepeater.model = 0

            var reportFields = visualReport.getReportFields()
            configFieldsRepeater.model = reportFields.length

            for (var i = 0; i < reportFields.length; ++i)
            {
                var fieldLine = reportFields[i]
                configFieldsRepeater.itemAt(i).loadValues(fieldLine[0], fieldLine[1], fieldLine[2])
            }
        }


        function storeFields(skipIdx = -1)
        {
            var fieldsList = []

            for (var i = 0; i < configFieldsRepeater.model; ++i)
            {
                if (i === skipIdx)
                    continue

                var name = configFieldsRepeater.itemAt(i).getName()
                var color = configFieldsRepeater.itemAt(i).getColor()
                var yCoef = configFieldsRepeater.itemAt(i).getYCoef()
                var fieldLine = [name, color, yCoef]

                fieldsList.push(fieldLine)
            }

            reportFieldsConfigPopup.storedFields = fieldsList
        }

        function loadFields()
        {
            for (var i = 0; i < reportFieldsConfigPopup.storedFields.length; ++i)
            {
                var fieldLine = reportFieldsConfigPopup.storedFields[i]
                configFieldsRepeater.itemAt(i).loadValues(fieldLine[0], fieldLine[1], fieldLine[2])
            }
        }



        Repeater
        {
            model: 3
            id: configFieldsRepeater

            RowLayout
            {
                y: index * 60

                function loadValues(name, color, coef)
                {
                    praatFieldName2.text = name
                    fieldCoef2.text = coef

                    for (var i = 0; i < reportFieldsConfigPopup.colorsNames.length; ++i)
                        if (reportFieldsConfigPopup.colorsNames[i] === color)
                            break;

                    fieldColor2.currentIndex = i
                }

                function getName() {
                    return praatFieldName2.text
                }

                function getColor() {
                    return fieldColor2.currentText
                }

                function getYCoef() {
                    return fieldCoef2.text
                }

                TextField
                {
                    id: praatFieldName2
                    implicitWidth: 400
                }

                ComboBox {
                    id: fieldColor2
                    model: reportFieldsConfigPopup.colorsNames
                }

                TextField {
                    id: fieldCoef2
                    validator: realValidator
                    text: "1,0"
                }

                RoundButton {
                    text: "-"

                    onClicked: {
                        reportFieldsConfigPopup.storeFields(index)
                        loadStoredFieldsTimer.start() //Hot fix for ReferenceError: xyz is not defined
                        configFieldsRepeater.model = configFieldsRepeater.model - 1 //WE got issue here as our current object is child
                    }
                }

                RoundButton {
                    text: "+"

                    onClicked:
                    {
                        reportFieldsConfigPopup.storeFields()
                        loadStoredFieldsTimer.start() //Hot fix for ReferenceError: xyz is not defined
                        configFieldsRepeater.model = configFieldsRepeater.model + 1 //WE got issue here as our current object is child
                    }
                }
            }
        }

        Timer
        {
            id: loadStoredFieldsTimer
            interval: 50
            running: false
            repeat: false
            onTriggered: {
                reportFieldsConfigPopup.loadFields()
            }
        }

        Button
        {
            text: "Add new"

            x: parent.width/2 - width/2 - 5
            y: parent.height - height - 5

            onClicked:
            {
                reportFieldsConfigPopup.storeFields()
                loadStoredFieldsTimer.start()
                configFieldsRepeater.model += 1 // //решением этого хотфикса выше можно сделать за счёт запуска таймера с инкрементом и декриментом, но это не критично
            }
        }


        Button
        {
            text: "Close"

            x: parent.width - width - 10
            y: parent.height - height - 5

            onClicked: reportFieldsConfigPopup.close()
        }

        Button
        {
            text: "Save"

            x: 10
            y: parent.height - height - 5

            onClicked:
            {
                reportFieldsConfigPopup.lastVisualReport.clearReportFields()

                for (var i = 0; i < configFieldsRepeater.model; ++i)
                {

                    var name = configFieldsRepeater.itemAt(i).getName()
                    var color = configFieldsRepeater.itemAt(i).getColor()
                    var yCoef = parseFloat(configFieldsRepeater.itemAt(i).getYCoef())

                    reportFieldsConfigPopup.lastVisualReport.addReportField(name, color, yCoef)
                }

                var type = reportFieldsConfigPopup.lastVisualReport.getType()
                reportFieldsConfigPopup.lastVisualReport.setType(type)

                reportFieldsConfigPopup.close()
            }
        }
    }




//=====Visual reports fields popups========================================================
//=========================================================================================


}
