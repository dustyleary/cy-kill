function emptyBoard(size) {
    var dots = '..........................................................................................................................................................................................................................................................................................................................................................................';
    return dots.substring(0, size*size);
}

function isValidMove(board, index, color) {
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
