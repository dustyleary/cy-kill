function Goban(args) {
    var self = this;
    this.el = $(args.el)[0] || elIsRequired;
    $(this.el).append("<label>Board Size: <select class='boardSize'></select></label>");
    $(this.el).append("<label>Default color: <select class='defaultColor'><option value='black'>Black</option><option value='white'>White</option></select></label>");
    var s = $('select.defaultColor', this.el)[0];
    $(s).change(function() {
        self.setBlacksTurn(this.value == 'black');
    });
    $(this.el).append("<label>Komi: "+this.defaultKomi+"</label>");
    $(this.el).append("<br>");
    var s = $('select.boardSize', this.el)[0];
    $([5,9,13,19]).each(function(i,v) {
        var o = $('<option>'+v+'x'+v+'</option>')[0];
        o.value = v;
        $(s).append(o);
    });
    $(s).change(function(a,b,c) {
        self.resetBoard(s.value);
    });
    $(this.el).append("<div class='boardContainer'><div class='board'></div></div>");
    $(this.el).append("<div class='boardState'>Copy & paste board state:<br><textarea class='boardText'></textarea></div>");
    var ta = $('textarea.boardText', this.el)[0];
    $(ta).bind('input', function() { self.onBoardTextChanged(); });

    this.resetBoard(9);
}

Goban.prototype.dotPoints = {
    9:[2,6],
    13:[3,6,9],
    19:[3,9,15],
};
Goban.prototype.defaultKomi = 6.5;

Goban.prototype.setBlacksTurn = function(b) {
    this.blacksTurn = b;
    var s = $('select.defaultColor', this.el)[0];
    if(b === false) {
        s.value = 'white';
    } else {
        s.value = 'black';
    }
}

Goban.prototype.resetBoard = function(size, boardState) {
    this.boardState = [];
    if(!boardState) {
        for(var i=0; i<size*size; i++) {
            this.boardState.push('.');
        }
    } else {
        for(var i=0; i<size*size; i++) {
            var ps = boardState[i];
            if(-1 == ".k#O".indexOf(ps)) {
                ps = '.';
            }
            this.boardState.push(ps);
        }
    }
    this.boardSize = size;
    this.setBlacksTurn(true);
    this.updateBoardText();
    this.buildBoard();
    this.updateBoardDisplay();
}

Goban.prototype.buildBoard = function() {
    var self = this;
    var s = $('select.boardSize', this.el)[0];
    s.value = this.boardSize;
    var b = $('.board', this.el)[0];
    b.innerHTML = '';
    function bindClick(x,y,el) {
        $(el).mousedown(function(e){
            self.clickPoint(x,y);
        });
    }
    this.boardPoints = [];
    for(var y=0; y<this.boardSize; y++) {
        var r = $("<div class='row'/>")[0];
        b.appendChild(r);
        for(var x=0; x<this.boardSize; x++) {
            var p = $("<div class='point'/>")[0];
            this.boardPoints.push(p);
            r.appendChild(p);
            if(x<(this.boardSize-1)) {
                r.appendChild($("<div class='hline'/>")[0]);
            }
            var dp = this.dotPoints[this.boardSize];
            if(dp) {
                if(-1 != dp.indexOf(x) && -1 != dp.indexOf(y)) {
                    if(this.boardSize != 13 || Math.abs(x-y)!=3) {
                        $(p).append("<div class='dot'/>");
                        $(p).addClass('asdf');
                    }
                }
            }
            $(p).append("<div class='white stone'/><div class='black stone'/><div class='ko'><div class='koshade'/><span class='kotext'>KO</span></div>");
            var pHitbox = $("<div class='hitbox'/>")[0];
            p.appendChild(pHitbox);
            bindClick(x,y,pHitbox);
        }
        if(y<(this.boardSize-1)) {
            var vlines = $("<div class='vlines'/>")[0];
            r.appendChild(vlines);
            for(var x=0; x<this.boardSize-1; x++) {
                vlines.appendChild($("<div class='vline'/>")[0]);
            }
        }
    }
}

Goban.prototype.pointClass = {
    '.':'empty',
    '#':'black',
    'O':'white',
    'k':'ko',
};

Goban.prototype.updateBoardDisplay = function() {
    for(var i=0; i<this.boardSize*this.boardSize; i++) {
        var pclass = this.pointClass[this.boardState[i]];
        $(this.boardPoints[i]).attr('class', 'point '+pclass);
    }
}

Goban.prototype.updateBoardText = function() {
    var bt = $('.boardText', this.el)[0];
    bt.value = this.boardState.join('');
}

Goban.prototype.clickPoint = function(x,y) {
    var ps = this.getPointState(x,y);
    var newPs = '.';
    if(false) {
    } else if(ps == '.') {
        newPs = this.blacksTurn?'#':'O';
    } else if(ps == '#') {
        newPs = 'O';
    } else if(ps == 'O') {
        newPs = 'k';
    }
    this.setPointState(x,y,newPs);
    this.updateBoardText();
    this.updateBoardDisplay();
}

Goban.prototype.getPointState = function(x,y,state) {
    return this.boardState[y*this.boardSize+x];
}

Goban.prototype.setPointState = function(x,y,state) {
    this.boardState[y*this.boardSize+x] = state;
}

Goban.prototype.onBoardTextChanged = function() {
    var ta = $('textarea.boardText', this.el)[0];
    var bt = ta.value;
    if(false) {
    } else if(bt.length >= 19*19) {
        this.resetBoard(19, bt);
    } else if(bt.length >= 13*13) {
        this.resetBoard(13, bt);
    } else if(bt.length >= 9*9) {
        this.resetBoard(9, bt);
    } else {
        this.resetBoard(5, bt);
    } 
}

