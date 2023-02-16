
//use cozy_chess::*;
#![warn(clippy::all, clippy::pedantic, clippy::nursery)]


use shakmaty::{CastlingMode, Chess, Move, Position, Role, san::San, fen::Fen, Rank, File, Color, Bitboard};

fn main() {

    let fen: Fen = "r3k2r/ppp2pp1/2qp4/4p3/4P1Pp/2P2N2/PP1P1P1P/R3K2R b KQkq g3 0 3".parse().unwrap();

    let pos: Chess = fen.into_position(CastlingMode::Standard).unwrap();

    let steps: Vec<Step>;

    steps = convert_san_to_steps("hxg3",
    pos,
    0.0,
    0.0);

    for step in steps  {
        print_step(step);
    }
}

fn update_board(position: &Chess, current_state: Bitboard){

}

fn convert_san_to_steps(san: &str, pos: Chess, captured_blacks: f64, captured_whites: f64) -> Vec<Step>{
    let san: San = san.parse().unwrap();

    let current_color: Color = pos.turn();
    let m = san.to_move(&pos).unwrap();

    let steps: Vec<Step>;
    steps = move_to_steps(m, current_color, captured_whites, captured_blacks);

    return steps;
}

fn move_to_steps(_move: Move, current_color: Color, captured_whites: f64, captured_blacks: f64) -> Vec<Step>
{
    let mut steps: Vec<Step> = Vec::new();

    let from_x: f64 = file_to_float(_move.from().unwrap().file());
    let from_y: f64 = rank_to_float(_move.from().unwrap().rank());
    let to_x: f64 = file_to_float(_move.to().file());
    let to_y: f64 = rank_to_float(_move.to().rank());
    
    if _move.is_castle(){ //from = king, to = rook
        let direction: f64;
        let offset: f64;
        let queenside_king: f64;
        if current_color == Color::White {direction = -0.5;} else {direction = 0.5;}
        if to_x == 8.0  {offset = -1.0; queenside_king = 0.0;} else {offset = 1.0; queenside_king = 1.0;}// king side castling; else queen side castling
        let engage: Step = Step { 
            x: (from_x), 
            y: (from_y), 
            magnet: (false) };
        
        let step1: Step = Step { 
            x: (to_x + offset + queenside_king), 
            y: (to_y), 
            magnet: (true) };

        let step2: Step = Step { 
            x: (to_x), 
            y: (to_y), 
            magnet: (false) };

        let step3: Step = Step { 
            x: (to_x), 
            y: (to_y + direction), 
            magnet: (true) };

        let step4: Step = Step { 
            x: (from_x - offset), 
            y: (to_y + direction), 
            magnet: (true) };
        
        let step5: Step = Step { 
            x: (from_x - offset), 
            y: (from_y), 
            magnet: (true) };
        
        steps.push(engage);
        steps.push(step1);
        steps.push(step2);
        steps.push(step3);
        steps.push(step4);
        steps.push(step5);
        return steps;
    }

    if _move.is_en_passant(){
        let offset: f64;
        if current_color == Color::White {offset = -1.0;} else {offset = 1.0;}
        let mut capture_moves: Vec<Step> = capture_piece(to_x, to_y + offset, current_color, captured_whites, captured_blacks);
        steps.append(&mut capture_moves);
    }

    if _move.is_capture() && !_move.is_en_passant() {
        let mut capture_moves: Vec<Step> = capture_piece(to_x, to_y, current_color, captured_whites, captured_blacks);
        steps.append(&mut capture_moves);
    }

    let engage: Step = Step { 
        x: (from_x), 
        y: (from_y), 
        magnet: (false) };
    
    steps.push(engage);

    if _move.role() == Role::Knight
    {
        let step1: Step = Step { 
            x: ((from_x + to_x)/2.0), 
            y: (from_y), 
            magnet: (true) };
        let step2: Step = Step { 
            x: ((from_x + to_x)/2.0), 
            y: (to_y), 
            magnet: (true) };
        let step3: Step = Step { 
            x: (to_x), 
            y: (to_y), 
            magnet: (true) };
        
        steps.push(step1);
        steps.push(step2);
        steps.push(step3);
        
    }
    //move to position
    else{
        let step: Step = Step { 
            x: (to_x), 
            y: (to_y), 
            magnet: (true) };
        steps.push(step);
    }

    return steps;
}

fn capture_piece(from_x: f64, from_y:f64, current_color:Color, captured_whites: f64, captured_blacks: f64) -> Vec<Step>
{
    let mut steps: Vec<Step> = Vec::new();
    let engage: Step = Step { 
        x: (from_x), 
        y: (from_y), 
        magnet: (false) };
    let direction: f64;

    let step1: Step;
    let step2: Step;
    let step3: Step;
    let step4: Step;
    
    if current_color == Color::White{ //BLACK IS CAPTURED
        if captured_blacks/2.0 < from_y {direction = -0.5;} else {direction = 0.5;}
        
        step1 = Step { 
            x: (from_x), 
            y: (from_y+direction), 
            magnet: (true) };
        
        step2 = Step { 
            x: (8.5), 
            y: (from_y+direction), 
            magnet: (true) };
        
        step3 = Step { 
            x: (8.5), 
            y: (0.5 + captured_blacks/2.0), 
            magnet: (true) };

        step4 = Step { 
            x: (9.0), 
            y: (0.5 + captured_blacks/2.0), 
            magnet: (true) };

    }
    else { //WHITE IS CAPTURED
        if 8.5-captured_whites/2.0 < from_y {direction = -0.5;} else {direction = 0.5;}
        
        step1 = Step { 
            x: (from_x), 
            y: (from_y+direction), 
            magnet: (true) };
        
        step2 = Step { 
            x: (0.5), 
            y: (from_y+direction), 
            magnet: (true) };
        
        step3 = Step { 
            x: (0.5), 
            y: (8.5 - captured_whites/2.0), 
            magnet: (true) };

        step4 = Step { 
            x: (0.0), 
            y: (8.5 - captured_whites/2.0), 
            magnet: (true) };
    }
    steps.push(engage);
    steps.push(step1);
    steps.push(step2);
    steps.push(step3);
    steps.push(step4);

    return steps;
}

struct Step
{
    x: f64,
    y: f64,
    magnet: bool,
}

fn print_step(step: Step){
    println!("x: {}", step.x);
    println!("y: {}", step.y);
    println!("magnet: {}", step.magnet);
}

fn rank_to_float(rank:shakmaty::Rank) -> f64{
    match rank {
        Rank::First   => return  1.0,
        Rank::Second  => return  2.0,
        Rank::Third   => return  3.0,
        Rank::Fourth  => return  4.0,
        Rank::Fifth   => return  5.0,
        Rank::Sixth   => return  6.0,
        Rank::Seventh => return  7.0,
        Rank::Eighth  => return  8.0,
    }
}

fn file_to_float(file: File) -> f64{
    match file {
        File::A => return 1.0,
        File::B => return 2.0,
        File::C => return 3.0,
        File::D => return 4.0,
        File::E => return 5.0,
        File::F => return 6.0,
        File::G => return 7.0,
        File::H => return 8.0,
    }
}



/*    
    let mut fullMove = deconstruct(moveSAN);

    // Start position
    let board = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2QKp/PPPBBPPP/R6R b kq - 0 1"
        .parse::<Board>()
        .unwrap();

    //used to denote situations when 2 pieces have to be moved like captures, promotions and castling
    let mut additionalMove = DeconstructedSan {
        piece: ' ',
        fromX: ' ',
        fromY: ' ',
        toX: ' ',
        toY: ' ',
        capture: false,
        castle: ' ',
    };
    if(fullMove.capture)
    {
        additionalMove.fromX = fullMove.toX;
        additionalMove.fromY = fullMove.toY;
        //'-' is supposed to denote taking the piece of the board (WIP)
        additionalMove.toX = '-';
        additionalMove.toY = '-';
    }

    //Get location of given type and colour
    let mut bitboard = board.colored_pieces(board.side_to_move(), charToPiece(fullMove.piece));

    //take care of the castling case
    if(fullMove.castle == 'K'){
        fullMove.piece = 'K';
        fullMove.fromX = 'e';
        fullMove.toX = 'g';
        if(board.side_to_move() == Color::White){
            fullMove.fromY = '1';
            fullMove.toY = '1';
        }
        else{
            fullMove.fromY = '8';
            fullMove.toY = '8';
        }

        additionalMove.piece = 'R';
        additionalMove.fromX = 'h';
        additionalMove.toX = 'f';
        if(board.side_to_move() == Color::White){
            additionalMove.fromY = '1';
            additionalMove.toY = '1';
        }
        else {
            additionalMove.fromY = '8';
            additionalMove.toY = '8';
        }
    }
    else if(fullMove.castle == 'Q'){
        fullMove.piece = 'K';
        fullMove.fromX = 'e';
        fullMove.toX = 'b';
        if(board.side_to_move() == Color::White){
            fullMove.fromY = '1';
            fullMove.toY = '1';
        }
        else{
            fullMove.fromY = '8';
            fullMove.toY = '8';
        }

        additionalMove.piece = 'R';
        additionalMove.fromX = 'a';
        additionalMove.toX = 'c';
        if(board.side_to_move() == Color::White){
            additionalMove.fromY = '1';
            additionalMove.toY = '1';
        }
        else {
            additionalMove.fromY = '8';
            additionalMove.toY = '8';
        }
    }
    else{
        //If file is already known, eliminate pieces that don't fit
        if(fullMove.fromX != ' '){
            bitboard = bitboard & charToFile(fullMove.fromX).bitboard();
        }  
        //If rank is already known, eliminate pieces that don't fit
        if(fullMove.fromY != ' '){
            bitboard = bitboard & charToRank(fullMove.fromY).bitboard();
        }  

        //get first square that might be the moving piece
        let mut x = bitboard.next_square();

        //get the destination square (known from SAN)
        let toSquare = charsToSquare(fullMove.toX,fullMove.toY);

        //Go through all given pieces until the correct one is found
        while(!x.is_none()){
            //create the move object using coordinates of potential starting square and destination square
            let mut newMove = Move{from: x.unwrap(), to: toSquare, promotion: None};

            //if the move is legal, it must be the correct piece
            if(board.is_legal(newMove))
            {
                println!("legal");
                //update the deconstructed move object
                fullMove.fromX = fileToChar(x.unwrap().file());
                fullMove.fromY = rankToChar(x.unwrap().rank());
                break;
            }
            println!("not legal");
            //the move is not legal, remove square from consideration
            bitboard = bitboard - x.unwrap().bitboard();
            //get next square
            x = bitboard.next_square();
        }
    }
    //print result out
    println!("Main move: ");
    printSAN(&fullMove);
    if(fullMove.capture || fullMove.castle != ' '){
        println!("Additional move: ");
        printSAN(&additionalMove);
    }
}

fn charsToSquare(file: char, rank:char) -> Square{
    return Square::new(charToFile(file), charToRank(rank));
}

fn rankToChar(rank:Rank) -> char{
    match rank {
        cozy_chess::Rank::First => return  '1',
        cozy_chess::Rank::Second => return  '2',
        cozy_chess::Rank::Third => return  '3',
        cozy_chess::Rank::Fourth => return  '4',
        cozy_chess::Rank::Fifth => return  '5',
        cozy_chess::Rank::Sixth => return  '6',
        cozy_chess::Rank::Seventh => return  '7',
        cozy_chess::Rank::Eighth => return  '8',
        _ => return  ' ',
    }
}

fn charToRank(rank:char) -> Rank{
    match rank{
        '1' => return Rank::First,
        '2' => return Rank::Second,
        '3' => return Rank::Third,
        '4' => return Rank::Fourth,
        '5' => return Rank::Fifth,
        '6' => return Rank::Sixth,
        '7' => return Rank::Seventh,
        '8' => return Rank::Eighth,
        _ => panic!(),
    }
}

fn fileToChar(file: File) -> char{
    match file {
        cozy_chess::File::A => return 'a',
        cozy_chess::File::B => return 'b',
        cozy_chess::File::C => return 'c',
        cozy_chess::File::D => return 'd',
        cozy_chess::File::E => return 'e',
        cozy_chess::File::F => return 'f',
        cozy_chess::File::G => return 'g',
        cozy_chess::File::H => return 'h',
        _ => return ' ',
    }
}

fn charToFile(file:char) ->File{
    match file {
        'a' => return File::A,
        'b' => return File::B,
        'c' => return File::C,
        'd' => return File::D,
        'e' => return File::E,
        'f' => return File::F,
        'g' => return File::G,
        'h' => return File::H,
        _ => panic!(),
    }
}

fn charToPiece(character: char) -> Piece{
    match character{
        'K' => return Piece::King,
        'Q' => return Piece::Queen,
        'N' => return Piece::Knight,
        'B' => return Piece::Bishop,
        'R' => return Piece::Rook,
        _ => return Piece::Pawn,
    }
}

fn printSAN(deconstructedSan: &DeconstructedSan)
{
    println!("piece: {}", deconstructedSan.piece);
    println!("fromX: {}", deconstructedSan.fromX);
    println!("fromY: {}", deconstructedSan.fromY);
    println!("toX: {}", deconstructedSan.toX);
    println!("toY: {}", deconstructedSan.toY);
    println!("capture: {}", deconstructedSan.capture);
    println!("castle: {}", deconstructedSan.castle);
}

fn deconstruct(moveSan: String) -> DeconstructedSan{
    let mut output = DeconstructedSan {
        piece: ' ',
        fromX: ' ',
        fromY: ' ',
        toX: ' ',
        toY: ' ',
        capture: false,
        castle: ' ',
    };

    let chars: Vec<char> = moveSan.chars().collect();
    let mut counter = 0;
    let length = chars.len();
    for c in chars{
        if  c == 'O' || c =='0'
        {
            if(length > 3){
                output.castle = 'Q';
            }
            else{
                output.castle = 'K';
            }
            break;
        }
        if c == 'x'
        {
            output.capture = true;
            counter = counter + 1;
            continue;
        }
        if c.is_uppercase()
        {
            output.piece = c;
            counter = counter + 1;
            continue;
        }
        if length - counter > 2
        {
            counter = counter + 1;
            if c.is_alphabetic()
            {
                output.fromX = c;
                continue;
            }
            else {
                output.fromY = c;
                continue;
            }
        }
        else{
            counter = counter + 1;
            if c.is_alphabetic()
            {
                output.toX = c;
                continue;
            }
            else {
                output.toY = c;
                continue;
            }
        }
        
    }
    if output.piece == ' ' {output.piece = 'P'}
    return output;

}
struct DeconstructedSan
{
    piece: char,
    fromX: char,
    fromY: char,
    toX: char,
    toY: char,
    capture: bool,
    castle: char,
}*/