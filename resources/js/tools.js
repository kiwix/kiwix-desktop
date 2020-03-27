function createDict(keys, values) {
    var d = {}
    for(var i=0; i<keys.length; i++) {
      d[keys[i]] = values[i];
    }
    return d;
}