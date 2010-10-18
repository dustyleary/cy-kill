function Board(size) {
    this.text = new Array(size*size);
    for(var i=0; i<size*size; i++) {
        this.text[i] = 0;
    }
    this.size = size;
}

Board.prototype.EMPTY = 0;
Board.prototype.BLACK = 1;
Board.prototype.WHITE = 2;

Board.prototype.bv = function(x,y) {
    return this.text[y*this.size+x];
}

Board.prototype.gl = function(x,y, c,l,v) {
    if(x<0 || y<0) return;
    if(x>=this.size || y>=this.size) return;
    if(-1 != v.indexOf(String.fromCharCode(x+97,y+97))) return;
    v.push(String.fromCharCode(x+97,y+97));
    var nc = this.bv(x,y);
    if(nc == this.EMPTY) l.push(String.fromCharCode(x+97,y+97));
    else if(nc == c) {
        this.gl(x-1,y, c,l,v);
        this.gl(x+1,y, c,l,v);
        this.gl(x,y-1, c,l,v);
        this.gl(x,y+1, c,l,v);
    }
}

Board.prototype.getLiberties = function(x,y) {
    if(x<0 || y<0) return [];
    if(x>=this.size || y>=this.size) return [];
    var c = this.bv(x,y);
    if(c == this.EMPTY) return [];
    var l = [];
    var v = [];
    this.gl(x-1,y, c,l,v);
    this.gl(x+1,y, c,l,v);
    this.gl(x,y-1, c,l,v);
    this.gl(x,y+1, c,l,v);
    return l;
}

Board.prototype.countLiberties = function(x,y) {
    return this.getLiberties(x,y).length;
}

Board.prototype.makeMove = function(color, x,y) {
    var index = y*this.size+x;
    this.text[index] = color;
}

Board.prototype.log = function() {
    for(var i=0; i<this.size; i++) {
        console.log(this.text.substr(i*this.size, this.size));
    }
}
