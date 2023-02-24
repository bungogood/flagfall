
//use cozy_chess::*;
#![warn(clippy::all, clippy::pedantic, clippy::nursery)]



use shakmaty::{CastlingMode, Chess, Move, Position, Role, san::San, fen::Fen, Rank, File, Color, Bitboard, Square};

fn main() {

    //for now chess games are going to start from the beginning, this FEN is used for testing
    let fen: Fen = "r3k2r/ppp2p2/2qp4/4pb2/4P1Pp/2P2N2/PP1P1PpP/R3K2R w KQkq - 0 3".parse().unwrap();

    let mut pos: Chess = fen.into_position(CastlingMode::Standard).unwrap();

    let mut state: State = State::Idle;
    let mut line;
    print_board_from_fen(pos.board().to_string());

    //Right not the program is set to loop through the input from the reed switches ONLY
    loop {
        line = String::new();
        let newstate = state.clone();
        let rgbstate = state.clone();
        print_state_name(state);

        //This is the output for LED MATRIX
        print_RGB(get_RGB(&pos, rgbstate));

        //This is input from REED SWITCHES
        std::io::stdin().read_line(&mut line).unwrap();
        if  line == "-1" {
            break;
        }
        else {
            let (_state,_move) = update_state(&pos, line.trim().parse::<u32>().unwrap(), newstate);
            state = _state;
            let copied_move = _move.clone();
            let copied_pos = pos.clone();
            if _move.is_some(){
                pos = copied_pos.play(&copied_move.unwrap()).unwrap();
                print_board_from_fen(pos.board().to_string());
            }

        }
    }

    //The input of SAN is gonna access through this method:
    //convert_san_to_steps(INPUT, pos, captured_blacks, captured_whites)
    //the method also gives an output for CORE-XY in the form of a list of structs
    //TODO: make sure that moves coming from SAN are committed by using Chess.play()
}

fn get_RGB(position: &Chess, state: State) -> RGB{
    let color = position.turn();
    let occupied = position.board().occupied();
    let enemies = position.them();
    match state {
        State::Idle => {
            let rgb = RGB{
                R: Bitboard::EMPTY,
                G: Bitboard::EMPTY,
                B: Bitboard::EMPTY,
            };
            rgb
        }
        State::FriendlyPU(square) => {
            let mut can_move_to: Bitboard;
            let mut is_promotion: bool = false;
            if position.board().role_at(square).unwrap() == Role::Pawn{
                let shift_direction: i32;
                if color.is_white() {shift_direction = 1} else {shift_direction = -1}
                can_move_to = Bitboard::from_square(square).shift(8 * shift_direction);
                if (square.rank() == Rank::Second && color.is_white() || square.rank() == Rank::Seventh && color.is_black()) && can_move_to.without(occupied).any() {
                    can_move_to = can_move_to.with(Bitboard::from_square(square).shift(16 * shift_direction));
                }
                can_move_to = can_move_to.without(occupied);

                if (square.rank() == Rank::Second && color.is_black() || square.rank() == Rank::Seventh && color.is_white()) && can_move_to.without(occupied).any() {
                    is_promotion = true;
                }
            }
            else{
                can_move_to = position.board().attacks_from(square).without(occupied);
            }

            let can_capture = position.board().attacks_from(square).intersect(enemies);
            
            let rgb: RGB;
            if is_promotion {
                rgb = RGB{
                    R: can_move_to.with(can_capture),
                    G: can_capture,
                    B: can_move_to,
                };
            }
            else{
                rgb = RGB{
                    R: can_capture,
                    G: can_move_to.with(can_capture),
                    B: Bitboard::EMPTY,
                };
            }
            rgb
        }
        State::EnemyPU(square) => {
            let attackers = position.board().attacks_to(square, color, occupied);
            let rgb = RGB{
                R: Bitboard::EMPTY,
                G: attackers,
                B: Bitboard::EMPTY,
            };
            rgb
        }
        State::FriendlyAndEnemyPU(_, enemy_square) => {
            let rgb = RGB{
                R: Bitboard::EMPTY,
                G: Bitboard::from_square(enemy_square),
                B: Bitboard::EMPTY,
            };
            rgb
        }
        State::Castling(_, rook_square) => {
            let target_square: Square;
            match color {
                Color::White => {
                    if rook_square == Square::A1{
                        target_square = Square::C1
                    }
                    else {
                        target_square = Square::G1
                    }
                }
                Color::Black => {
                    if rook_square == Square::A8{
                        target_square = Square::C8
                    }
                    else {
                        target_square = Square::G8
                    }
                }
                
            }

            let rgb = RGB{
                R: Bitboard::from_square(target_square),
                G: Bitboard::EMPTY,
                B: Bitboard::from_square(target_square),
            };
            rgb
        }
        State::CastlingPutRookDown(_, _, target_square) => {
            let rgb = RGB{
                R: Bitboard::from_square(target_square),
                G: Bitboard::EMPTY,
                B: Bitboard::from_square(target_square),
            };
            rgb
        }
        State::InvalidPiecePU(_, square) => {
            let rgb = RGB{
                R: Bitboard::from_square(square),
                G: Bitboard::EMPTY,
                B: Bitboard::EMPTY,
            };
            rgb
        }
        State::InvalidMove(_, square) => {
            let rgb = RGB{
                R: Bitboard::from_square(square),
                G: Bitboard::EMPTY,
                B: Bitboard::EMPTY,
            };
            rgb
        }
        State::Error => {
            let rgb = RGB{
                R: Bitboard::FULL,
                G: Bitboard::EMPTY,
                B: Bitboard::EMPTY,
            };
            rgb
        }
        
    }
}

struct RGB{
    R: Bitboard,
    G: Bitboard,
    B: Bitboard,
} 

fn print_RGB(rgb: RGB){
    print_bitboard(rgb.R);
    print_bitboard(rgb.G);
    print_bitboard(rgb.B);
}

fn update_state(position: &Chess, instruction: u32, state: State) -> (State, Option<Move>){
    let color = position.turn();
    let square = Square::new(instruction);
    let occupied = position.board().occupied();
    let friendlies = position.us();
    let enemies = position.them();

    match state {
        State::Idle =>
        {
            if friendlies.contains(square) {
                (State::FriendlyPU(square),None)
            }
            else if enemies.contains(square) {
                if position.board().attacks_to(square, color, occupied).any(){
                    (State::EnemyPU(square),None)
                }
                else{
                    (State::InvalidPiecePU(None, square),None)
                }
            }
            else {
                (State::Error,None)
            }

        },
        State::FriendlyPU(prev_square) =>
        {  
            let role_picked_up = position.board().role_at(prev_square).unwrap();
            let can_capture = position.board().attacks_from(prev_square).intersect(enemies);
            if prev_square == square {
                (State::Idle,None)
            }
            else if role_picked_up == Role::Rook && position.board().role_at(square).is_some() && position.board().role_at(square).unwrap() == Role::King{ //castling
                let _move = Move::Castle { king: square, rook: prev_square };
                if position.is_legal(&_move){
                    (State::Castling(square, prev_square),None)
                }
                else {
                    (State::InvalidPiecePU(Some(prev_square), square),None)
                }
            }
            else if role_picked_up == Role::King && position.board().role_at(square).is_some() && position.board().role_at(square).unwrap() == Role::Rook { //castling
                let _move = Move::Castle { king: prev_square, rook: square };
                if position.is_legal(&_move){
                    (State::Castling(prev_square, square),None)
                }
                else {
                    (State::InvalidPiecePU(Some(prev_square), square),None)
                }
            }
            else if friendlies.contains(square) || (enemies.contains(square) && !can_capture.contains(square)){
                (State::InvalidPiecePU(Some(prev_square), square),None)
            }
            else if can_capture.contains(square){
                (State::FriendlyAndEnemyPU(prev_square, square),None)
            }
            else if role_picked_up == Role::Pawn && (square.rank() == Rank::First || square.rank() == Rank::Eighth) { //promotions
                let _move = Move::Normal { role: (Role::Pawn),
                    from: (prev_square),
                    capture: (None),
                    to: (square),
                    promotion: (Some(Role::Queen)) }; //Right now we're just assuming the player will promote to queen
                println!("PROMOTED");
                (State::Idle,Some(_move))
            }
            else{
                let _move = Move::Normal { role: (role_picked_up),
                    from: (prev_square),
                    capture: (None),
                    to: (square),
                    promotion: (None) };
                if position.is_legal(&_move){
                    println!("MOVE COMMITTED");
                    (State::Idle,Some(_move))
                }
                else{(State::InvalidMove(prev_square, square), None)}
            }
        },
        State::EnemyPU(prev_square) =>
        {
            if prev_square == square {
                (State::Idle,None)
            }
            else if !position.board().attacks_to(prev_square, color, occupied).contains(square) || enemies.contains(square)
                || (position.board().role_at(square).unwrap() == Role::King && position.king_attackers(prev_square, color.other(), occupied).any()){
                (State::InvalidPiecePU(Some(prev_square), square),None)
            }
            else if position.board().attacks_to(prev_square, color, occupied).contains(square) {
                (State::FriendlyAndEnemyPU(square, prev_square),None)
            }
            else {
                (State::Error,None)
            }
        },
        State::FriendlyAndEnemyPU(prev_friendly_square, prev_enemy_square) =>
        {
            let role_picked_up = position.board().role_at(prev_friendly_square).unwrap();
            if square == prev_friendly_square{
                (State::EnemyPU(prev_enemy_square),None)
            }
            else if square == prev_enemy_square {
                println!("CAPTURED");
                if role_picked_up == Role::Pawn && (square.rank() == Rank::First || square.rank() == Rank::Eighth) {
                    println!("PROMOTED");
                    let _move = Move::Normal { role: (role_picked_up),
                        from: (prev_friendly_square),
                        capture: (position.board().role_at(prev_enemy_square)),
                        to: (square),
                        promotion: (Some(Role::Queen)) }; //assuming player will pick queen
                    (State::Idle,Some(_move))
                }
                else{
                    let _move = Move::Normal { role: (role_picked_up),
                        from: (prev_friendly_square),
                        capture: (position.board().role_at(prev_enemy_square)),
                        to: (square),
                        promotion: (None) };
                    (State::Idle,Some(_move))
                }
            }
            else {
                (State::Error,None)
            }
        },
        State::Castling(king_square, rook_square) => //make it more robust
        {
            match color {
                Color::White =>
                {
                    if rook_square.file() == File::A { //queen side
                        if square == Square::C1{
                            (State::CastlingPutRookDown(king_square, rook_square, Square::D1),None)
                        }
                        else {(State::Error, None)}
                    }
                    else{ //king side
                        if square == Square::G1{
                            (State::CastlingPutRookDown(king_square, rook_square, Square::F1),None)
                        }
                        else {(State::Error, None)}
                    }
                }
                Color::Black =>
                {
                    if rook_square.file() == File::A { //queen side
                        if square == Square::C8{
                            (State::CastlingPutRookDown(king_square, rook_square, Square::D8),None)
                        }
                        else {(State::Error, None)}
                    }
                    else{ //king side
                        if square == Square::G8{
                            (State::CastlingPutRookDown(king_square, rook_square, Square::F8),None)
                        }
                        else {(State::Error, None)}
                    }
                }
            }
        }
        State::CastlingPutRookDown(king_square,rook_square ,target_square) =>
        {
            if square == target_square {
                let _move = Move::Castle { king: king_square, rook: rook_square };
                (State::Idle, Some(_move))
            }
            else{
                (State::Error, None)
            }
        }
        State::InvalidPiecePU(prev_prev_square, prev_square) =>
        {
            if square == prev_square && prev_prev_square.is_none(){
                (State::Idle,None)
            }
            else if square == prev_square && friendlies.contains(prev_prev_square.unwrap()){
                (State::FriendlyPU(prev_prev_square.unwrap()),None)
            }
            else if square == prev_square && enemies.contains(prev_prev_square.unwrap()) {
                (State::EnemyPU(prev_prev_square.unwrap()),None)
            }
            else {
                (State::Error,None)
            }
        },
        State::InvalidMove(prev_prev_square, prev_square) =>
        {
            if square == prev_square {
                (State::FriendlyPU(prev_prev_square),None)
            }
            else {
                (State::Error,None)
            }
        },
        State::Error =>
        {
            (State::Error,None)
        },
    }
}

#[derive(Clone)]
enum State{
    Idle,
    FriendlyPU(Square),
    EnemyPU(Square),
    FriendlyAndEnemyPU(Square, Square),
    Castling(Square, Square),
    CastlingPutRookDown(Square, Square, Square),
    InvalidPiecePU(Option<Square>, Square),
    InvalidMove(Square, Square),
    Error,
}

fn print_state_name(state: State){
    match state {
        State::Idle => println!("Idle"),
        State::FriendlyPU(_) => println!("FriendlyPU"),
        State::EnemyPU(_) => println!("EnemyPU"),
        State::FriendlyAndEnemyPU(_,_) => println!("FriendlyAndEnemyPU"),
        State::Castling(_,_) => println!("Castling"),
        State::CastlingPutRookDown(_,_,_) => println!("CastlingPutRookDown"),
        State::InvalidPiecePU(_,_) => println!("InvalidPiecePU"),
        State::InvalidMove(_,_) => println!("InvalidMove"),
        State::Error => println!("Error"),
    }
}

fn print_board_from_fen(fen: String){
    let mut output: String = String::new();
    let mut counter = 0;
    output.push(' ');
    for c in fen.chars() {
        if counter == 8
        {
            counter = 0;
            output.push_str("\n");
        }
        match c {
            'r' => {output.push_str("r "); counter += 1},
            'R' => {output.push_str("R "); counter += 1},
            'n' => {output.push_str("n "); counter += 1},
            'N' => {output.push_str("N "); counter += 1},
            'b' => {output.push_str("b "); counter += 1},
            'B' => {output.push_str("B "); counter += 1},
            'q' => {output.push_str("q "); counter += 1},
            'Q' => {output.push_str("Q "); counter += 1},
            'k' => {output.push_str("k "); counter += 1},
            'K' => {output.push_str("K "); counter += 1},
            'p' => {output.push_str("p "); counter += 1},
            'P' => {output.push_str("P "); counter += 1},
            '1' => {output.push_str(". "); counter += 1},
            '2' => {output.push_str(". . "); counter += 2},
            '3' => {output.push_str(". . . "); counter += 3},
            '4' => {output.push_str(". . . . "); counter += 4},
            '5' => {output.push_str(". . . . . "); counter += 5},
            '6' => {output.push_str(". . . . . . "); counter += 6},
            '7' => {output.push_str(". . . . . . . "); counter += 7},
            '8' => {output.push_str(". . . . . . . . "); counter += 8},
            _ => {output.push(' '); counter += 0},
        }
    }
    println!("{}", output.as_str());
}

fn print_bitboard(bitboard: Bitboard){
    let y = format!("{bitboard:064b}");

    let mut output : String = String::new();
    let mut counter = 0;
    let mut line = String::new();
    for a in y.chars(){
    
        if counter % 8 == 0 {
            output.push_str(line.chars().rev().collect::<String>().as_str());
            //print!("{}", line.as_str());
            output.push_str("\n");
            line = String::new();
        }
        line.push(a);
        line.push(' ');
        counter += 1;
    
    }
    output.push_str(line.chars().rev().collect::<String>().as_str());
    println!("{}", output.as_str());
}

fn convert_san_to_steps(san: &str, pos: Chess, captured_blacks: f64, captured_whites: f64) -> Vec<Step>{
    let san: San = san.parse().unwrap();

    let current_color: Color = pos.turn();
    let m = san.to_move(&pos).unwrap();

    let steps: Vec<Step>;
    steps = move_to_steps(m, current_color, captured_whites, captured_blacks);

    return steps;
}

fn move_to_steps(_move: Move, current_color: Color, captured_whites: f64, captured_blacks: f64) -> Vec<Step>{
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

fn capture_piece(from_x: f64, from_y:f64, current_color:Color, captured_whites: f64, captured_blacks: f64) -> Vec<Step>{
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

struct Step{
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
