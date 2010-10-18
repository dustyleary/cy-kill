function Board(size, text) {
    if(!text) {
        var dots = '............................................................................................................................................................................................................................................................................................................................................................................';
        /*while(dots.length<size*size) {
            dots += dots;
        }*/
        text = dots.substr(0, size*size);
    }
    this.text = text;
    this.size = size;
}

Board.prototype.EMPTY = '.';
Board.prototype.BLACK = '#';
Board.prototype.WHITE = 'O';
Board.prototype.KILLEDWHITE = 'W';
Board.prototype.KILLEDBLACK = 'B';
Board.prototype.KILLED = {};
Board.prototype.KILLED[Board.prototype.BLACK] = Board.prototype.KILLEDBLACK;
Board.prototype.KILLED[Board.prototype.WHITE] = Board.prototype.KILLEDWHITE;
Board.prototype.KO = 'k';

Board.prototype.bv = function(x,y) {
    return this.text[y*this.size+x];
}

Board.prototype.isEmpty = function(c) {
    return false
        || c==this.EMPTY
        || c==this.KO
        || c==this.KILLED[this.WHITE]
        || c==this.KILLED[this.BLACK]
        ;
}

Board.prototype.gl = function(x,y, c,l,v) {
    if(x<0 || y<0) return;
    if(x>=this.size || y>=this.size) return;
    if(-1 != v.indexOf(String.fromCharCode(x+97,y+97))) return;
    v.push(String.fromCharCode(x+97,y+97));
    var nc = this.bv(x,y);
    if(this.isEmpty(nc)) l.push(String.fromCharCode(x+97,y+97));
    else if(nc == c) {
        this.gl(x-1,y, c,l,v);
        this.gl(x+1,y, c,l,v);
        this.gl(x,y-1, c,l,v);
        this.gl(x,y+1, c,l,v);
    }
}

Board.prototype.getLiberties = function(x,y) {
    if(x<0 || y<0) return null;
    if(x>=this.size || y>=this.size) return null;
    var c = this.bv(x,y);
    if(this.isEmpty(c)) return null;
    var l = [];
    var v = [];
    this.gl(x-1,y, c,l,v);
    this.gl(x+1,y, c,l,v);
    this.gl(x,y-1, c,l,v);
    this.gl(x,y+1, c,l,v);
    return [c,l];
}

Board.prototype.countLiberties = function(x,y) {
    var gl = this.getLiberties(x,y);
    if(gl === null) return 0;
    return gl[1].length;
}

Board.prototype.setPoint = function(state, x,y) {
    if(state === null) throw "bad";
    if(state === undefined) throw "bad";
    var index = y*this.size+x;
    this.text = this.text.substr(0,index)+state+this.text.substr(index+1);
}

Board.prototype.killGroup = function(color, x,y) {
    if(x<0 || y<0) return 0;
    if(x>=this.size || y>=this.size) return 0;
    var c = this.bv(x,y);
    if(c != color) return 0;
    this.setPoint(this.KILLED[color], x,y);
    var n = 1;
    n += this.killGroup(color, x-1,y);
    n += this.killGroup(color, x+1,y);
    n += this.killGroup(color, x,y-1);
    n += this.killGroup(color, x,y+1);
    return n;
}

Board.prototype.killGroupIfDead = function(color, x,y) {
    if(x<0 || y<0) return 0;
    if(x>=this.size || y>=this.size) return 0;
    var c = this.bv(x,y);
    if(c != color) return 0;
    var gl = this.getLiberties(x,y);
    if(gl === null) return 0;
    if(gl[1].length == 0) { //no liberties
        return this.killGroup(color, x,y);
    }
    return 0;
}

Board.prototype.makeMove = function(color, x,y) {
    this.setPoint(color, x,y);
    if(color != this.BLACK && color != this.WHITE) {
        return;
    }
    //remove old ko
    this.text = this.text.replace(this.KO, this.EMPTY);
    //kill opposing groups
    var killColor = color == this.BLACK ? this.WHITE : this.BLACK;
    var killedOther = 0;
    killedOther += this.killGroupIfDead(killColor, x-1, y);
    killedOther += this.killGroupIfDead(killColor, x+1, y);
    killedOther += this.killGroupIfDead(killColor, x, y-1);
    killedOther += this.killGroupIfDead(killColor, x, y+1);
    //mark ko
    if(killedOther == 1) {
        var gl = this.getLiberties(x,y);
        if(gl && gl[1].length == 1) {
            //I have just made a move that killed, and I am
            //in atari...  so I mark a ko
            var kx = gl[1][0].charCodeAt(0)-97;
            var ky = gl[1][0].charCodeAt(1)-97;
            this.setPoint(this.KO, kx,ky);
        }
    }
    //suicide
    var killedSelf = this.killGroupIfDead(color, x, y);
    //remove dead stones
    this.text = this.text.replace(/[WB]/g, this.EMPTY);
    var killedBlack = color==this.BLACK ? killedSelf : killedOther;
    var killedWhite = color==this.WHITE ? killedSelf : killedOther;
    return [killedBlack, killedWhite];
}

Board.prototype.clone = function() {
    return new Board(this.size, this.text);
}

Board.prototype.cg = function(x,y, c,v) {
    if(x<0 || y<0) return 0;
    if(x>=this.size || y>=this.size) return 0;
    if(-1 != v.indexOf(String.fromCharCode(x+97,y+97))) return 0;
    v.push(String.fromCharCode(x+97,y+97));
    var nc = this.bv(x,y);
    if(this.isEmpty(nc)) return 0;
    else if(nc == c) {
        return 1
            + this.cg(x-1,y, c,v)
            + this.cg(x+1,y, c,v)
            + this.cg(x,y-1, c,v)
            + this.cg(x,y+1, c,v)
            ;
    }
    return 0;
}
Board.prototype.countGroup = function(x,y) {
    var c = this.bv(x,y);
    return this.cg(x,y, this.bv(x,y),[]);
}

Board.prototype.testAtari = function(color, x,y) {
    if(this.bv(x,y) != color) return 0;
    if(this.countLiberties(x,y) == 1) {
        return this.countGroup(x,y);
    }
    return 0;
}

Board.prototype.fc = function(x,y, xt,yt, c,v) {
    if(x<0 || y<0) return false;
    if(x>=this.size || y>=this.size) return false;
    if(x==xt && y==yt) return true;
    if(-1 != v.indexOf(String.fromCharCode(x+97,y+97))) return false;
    v.push(String.fromCharCode(x+97,y+97));
    var nc = this.bv(x,y);
    if(nc != c) return false;
    return false
        || this.fc(x-1,y, xt,yt, c,v)
        || this.fc(x+1,y, xt,yt, c,v)
        || this.fc(x,y-1, xt,yt, c,v)
        || this.fc(x,y+1, xt,yt, c,v)
        ;
}

Board.prototype.isConnected = function(x1,y1, x2,y2) {
    var c = this.bv(x1,y1);
    if(c != this.bv(x2,y2)) return false;
    return this.fc(x1,y1, x2,y2, c,[]);
}

Board.prototype.isSuicide = function(color, x,y, b) {
    if(!b) b = this.clone();
    var captures = b.makeMove(color, x,y);
    return false
        || (color==this.BLACK && captures[0])
        || (color==this.WHITE && captures[1])
        ;
}

Board.prototype.atariInfo = function(color, x,y) {
    var b = this.clone();
    if(this.isSuicide(color, x,y,b)) {
        //suicide can never atari
        return [0,0];
    }
    var killColor = color == this.BLACK ? this.WHITE : this.BLACK;
    var groups = 0;
    var points = 0;
    var tried = [];
    function tryit(x,y) {
        if(x<0 || y<0) return;
        if(x>=b.size || y>=b.size) return;

        for(var i=0;i<tried.length;i++) {
            var tp = tried[i];
            if(b.isConnected(tp[0],tp[1], x,y)) return;
        }
        if(b.bv(x,y) == killColor) {
            tried.push([x,y]);
        }
        var c = b.testAtari(killColor, x,y);
        if(c) {
            groups += 1;
            points += c;
        }
    }
    tryit(x-1,y);
    tryit(x+1,y);
    tryit(x,y-1);
    tryit(x,y+1);

    return [groups, points];
}

Board.prototype.log = function() {
    for(var i=0; i<this.size; i++) {
        console.log(this.text.substr(i*this.size, this.size));
    }
}
