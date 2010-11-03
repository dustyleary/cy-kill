function Goban(args) {
    var self = this;
    this.el = $(args.el)[0] || elIsRequired;
    $(this.el).append("<div class='boardContainer'><div class='board'></div></div>");
}

Goban.prototype.dotPoints = {
    9:[2,6],
    13:[3,6,9],
    19:[3,9,15],
};

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
    this.size = size;
    this.buildBoard();
    this.updateBoardDisplay();
}

Goban.prototype.buildBoard = function() {
    var self = this;
    var b = $('.board', this.el)[0];
    b.innerHTML = '';
    function bindClick(x,y,el) {
        $(el).mousedown(function(e){
            self.clickPoint(x,y);
        });
    }
    this.boardPoints = [];
    for(var y=0; y<this.size; y++) {
        var r = $("<div class='row'/>")[0];
        b.appendChild(r);
        for(var x=0; x<this.size; x++) {
            var p = $("<div class='point'/>")[0];
            this.boardPoints.push(p);
            r.appendChild(p);
            if(x<(this.size-1)) {
                r.appendChild($("<div class='hline'/>")[0]);
            }
            var dp = this.dotPoints[this.size];
            if(dp) {
                if(-1 != dp.indexOf(x) && -1 != dp.indexOf(y)) {
                    if(this.size != 13 || Math.abs(x-y)!=3) {
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
        if(y<(this.size-1)) {
            var vlines = $("<div class='vlines'/>")[0];
            r.appendChild(vlines);
            for(var x=0; x<this.size-1; x++) {
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
    for(var i=0; i<this.size*this.size; i++) {
        var pclass = this.pointClass[this.boardState[i]];
        $(this.boardPoints[i]).attr('class', 'point '+pclass);
    }
}

Goban.prototype.clickPoint = function(x,y) {
    $(this).trigger('pointClicked', {x:x, y:y});
}

Goban.prototype.getPointState = function(x,y,state) {
    return this.boardState[y*this.size+x];
}

Goban.prototype.setPointState = function(x,y,state) {
    this.boardState[y*this.size+x] = state;
}

