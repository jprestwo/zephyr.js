var fs = require('fs');

print("Hello Flash");

var file = fs.open("file_name");
var file1 = fs.open("file_name1");

print("File = " + file);

fs.write(file, "test string", 0, 11);
var test1 = null;
var test = fs.read(file, test1, 0, 11);

fs.write(file1, "another_test", 0, 12);
var test2 = null;
var test2_ret = fs.read(file1, test2, 0, 12);

print("Read back = " + test);
print("Read back 2 = " + test2_ret);