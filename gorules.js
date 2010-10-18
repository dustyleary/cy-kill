function Board(size) {
    var dots = '..........................................................................................................................................................................................................................................................................................................................................................................';
    while(dots.length < size*size) { dots += dots; }
    this.text = dots.substr(0, size*size);
    this.size = size;
}

Board.prototype.BLACK = '#';

Board.prototype.bv = function(x,y) {
    return this.text[y*this.size+x];
}

Board.prototype.gl = function(x,y, c,l,v) {
    if(x<0 || y<0) return;
    if(x>=this.size || y>=this.size) return;
    if(-1 != v.indexOf(String.fromCharCode(x+97,y+97))) return;
    v.push(String.fromCharCode(x+97,y+97));
    var nc = this.bv(x,y);
    if(nc == '.') l.push(String.fromCharCode(x+97,y+97));
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
    if(c == '.') return [];
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
    var t = this.text;
    t = t.substr(0,index)+color+t.substr(index+1);
    this.text = t;
}

Board.prototype.log = function() {
    for(var i=0; i<this.size; i++) {
        console.log(this.text.substr(i*this.size, this.size));
    }
}

/*
function isValidMove(board, position, color) {
    if(board[index] != '.') return false;
    return true;
}

function makeMove(board, index, color) {
    if(!isValidMove(board, index, color)) {
        throw 'illegal move';
    }
    var newBoard = board.substring(0,index)+color+board.substring(index+1);
    return newBoard;
}

function isEmptyPoint(board, index) {
    if(index<1) return false;
    if(index>=board.length) return false;
    var c = board[index];
    return c=='.' || c=='k';
}

function boardSize(board) {
    switch(board.length) {
        case 361: return 19;
        case 169: return 13;
        case 81: return 9;
        case 25: return 5;
    }
    return Math.sqrt(board.length);
}

function isSuicide(board, index, color) {
    return !(false
        || isEmptyPoint(board, index-1)
        || isEmptyPoint(board, index+1)
        || isEmptyPoint(board, index-boardSize(board))
        || isEmptyPoint(board, index+boardSize(board))
        );
}
*/
