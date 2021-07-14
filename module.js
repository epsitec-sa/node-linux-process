const addon = require("./build/Release/module");

function isProcessRunning(processPid, processName) {
  const res = addon.is_process_running(processPid, processName);

  if (res !== 1 && res !== 0) {
    throw `Could not check for running process: ${res}`;
  }

  return res === 1 ? true : false;
}

module.exports.isProcessRunning = isProcessRunning;
