/*
	This is a very basic example script that can be parsed by the
	built-in ECMAScript 5.1 engine (duktape).
*/

// 6x² + 11x - 35 = 0
var a = 6;
var b = 11;
var c = -35;

// Calculate quadratic roots
var pos_root = ((-1 * b) + Math.sqrt(Math.pow(b, 2) - (4 * a * c))) / (2 * a);
var neg_root = ((-1 * b) - Math.sqrt(Math.pow(b, 2) - (4 * a * c))) / (2 * a);

// Write the output to a file
var output = "The roots are: " + pos_root + " and " + neg_root;
writeFile("roots.txt", output);

// Read from the file and print
print(readFile("roots.txt"));