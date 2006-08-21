/**
 *  This file provides support functions that act as syntactic sugar to
 * make Javascript's object-oriented features a little more straightforward
 * to use. */
 
//  Takes a name and a function, and adds them to a function's prototype object.
// Returns this.
Function.prototype.method = function (name, func) {
    this.prototype[name] = func;
    return this;
};

//  Indicates that one class inherits from another. Should be called after both classes 
// are defined, but before the inheriting class's methods are added.
Function.method('inherits', function (parent) {
    var d = 0, p = (this.prototype = new parent());
    this.method('base', function base(name) {
        var f, r, t = d, v = parent.prototype;
        if (t) {
            while (t) {
                v = v.constructor.prototype;
                t -= 1;
            }
            f = v[name];
        } else {
            f = p[name];
            if (f == this[name]) {
                f = v[name];
            }
        }
        d += 1;
        r = f.apply(this, Array.prototype.slice.apply(arguments, [1]));
        d -= 1;
        return r;
    });
    return this;
});

//  Loops through the arguments. For each name, it copies a member from the parent's 
// prototype to the new class's prototype.  Useful for object augmentation, where you want
// only the methods and properties useful to you from some other object. (You will be assimilated.)
Function.method('assimilate', function (parent) {
    for (var i = 1; i < arguments.length; i += 1) {
        var name = arguments[i];
        this.prototype[name] = parent.prototype[name];
    }
    return this;
});

/// Tests the value test.  If it's undefined, it returns the default, other wise it returns value.
function SafeDefault(testv, value, the_default)
{
	if (testv==undefined) return the_default;
	
	return value;
}

/// Tests the value test.  If it's undefined, it returns the default, otherwise it evaluates value and returns it.
function SafeDefaultEval(testv, value, the_default)
{
	if (testv==undefined) return the_default;
	
	return eval(value);
}
