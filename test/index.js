/*global describe,it*/

var spawn = require("child_process").spawn;
var assert = require("assert");
var lib = require("../");

describe("IsProcessRunning", function () {
  it("should be running", function () {
    const child = spawn("Terminal");

    assert.strictEqual(lib.isProcessRunning(child.pid, "Terminal"), true);

    child.stdin.pause();
    child.kill();
  });

  it("should not be running", function () {
    assert.strictEqual(lib.isProcessRunning(1234, "Terminal"), false);
  });

  it("should external process be running", function () {
    assert.strictEqual(lib.isProcessRunning(361, "Terminal"), true);
  });
});
