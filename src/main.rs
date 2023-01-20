mod bong;

fn main() {}

#[test]
fn lexer_a() {
    use crate::bong::lexer_a::{lex, Token};
    let test_text = "text.title {
    // text { \"hello world\" }
    \"hello world\"
}";
    let tokens = lex(test_text.to_string());
    assert_eq!(
        tokens,
        Ok(vec![
            Token::SlWhitespace("".to_string()),
            Token::Name("text".to_string()),
            Token::Class(".title".to_string()),
            Token::SlWhitespace(" ".to_string()),
            Token::LBrace("{".to_string()),
            Token::MlWhitespace("\n    ".to_string()),
            Token::SlComment("// text { \"hello world\" }".to_string()),
            Token::MlWhitespace("\n    ".to_string()),
            Token::String("\"hello world\"".to_string()),
            Token::MlWhitespace("\n".to_string()),
            Token::RBrace("}".to_string()),
        ])
    )
}

#[test]
#[rustfmt::skip]
fn lexer_b() {
    use crate::bong::lexer_b::{lex, Location, Token, TokenType};
    let test_text = "text.title {
    // text { \"hello world\" }
    \"hello world\"
}";
    let tokens = lex(test_text)
        .unwrap();
    let tokens_and_values: Vec<(&Token, &str)> = tokens
        .iter()
        .map(|token| (token, token.value(test_text)))
        .collect();

    assert_eq!(tokens_and_values, vec![
        (&Token { token_type: TokenType::Name("text".to_owned()), location: Location { index: 0, line: 1, col: 1 }, length: 4 }, "text"),
        (&Token { token_type: TokenType::Class("title".to_owned()), location: Location { index: 4, line: 1, col: 5 }, length: 6 }, ".title"),
        (&Token { token_type: TokenType::SlWhitespace, location: Location { index: 10, line: 1, col: 11 }, length: 1 }, " "),
        (&Token { token_type: TokenType::LBrace, location: Location { index: 11, line: 1, col: 12 }, length: 1 }, "{"),
        (&Token { token_type: TokenType::SlWhitespace, location: Location { index: 12, line: 2, col: 1 }, length: 5 }, "\n    "),
        (&Token { token_type: TokenType::SlComment, location: Location { index: 17, line: 2, col: 6 }, length: 25 }, "// text { \"hello world\" }"),
        (&Token { token_type: TokenType::SlWhitespace, location: Location { index: 42, line: 3, col: 1 }, length: 5 }, "\n    "),
        (&Token { token_type: TokenType::String("hello world".to_owned()), location: Location { index: 47, line: 3, col: 6 }, length: 13 }, "\"hello world\""),
        (&Token { token_type: TokenType::SlWhitespace, location: Location { index: 60, line: 4, col: 1 }, length: 1 }, "\n"),
        (&Token { token_type: TokenType::RBrace, location: Location { index: 61, line: 4, col: 2 }, length: 1 }, "}"),
    ])
}
