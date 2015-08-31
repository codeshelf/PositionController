include(".cre.js");

var text = new gui.Text("Codeshelf");
var rawText = "";
gui.statusText = null;
gui.show(text);

reader.onCommandFinishOld = reader.onCommandFinish;
reader.onCommandFinish = function(commandSuccess, responseType, responseData) {
	reader.onCommandFinishOld(commandSuccess, responseType, responseData);
}

reader.onCommandOld = reader.onCommand;
reader.onCommand = function(commandType, commandData) {

	reader.onCommandOld(commandType, commandData);
	if (commandType == '|') {
		
		if (commandData == "clear") {
			rawText = "";
			text = new gui.Text(rawText);
		} else {
			rawText = rawText.concat(commandData + '\n');
			text = new gui.Text(rawText);
		}
		
		gui.statusText = null;
		gui.show(text);

		return false;
	} else {
		lastCmdType = '';
		return true;
	}

}

ready.showOld = ready.show;
ready.show = function(text, busy) {
	// Do nothing
}

reader.onDecodeOld = reader.onDecode;
reader.onDecode = function(decode) {
	return reader.onDecodeOld(decode);
}