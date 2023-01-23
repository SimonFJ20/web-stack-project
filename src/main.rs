use std::time::Duration;

use sdl2::{
    event::Event, keyboard::Keycode, pixels::Color, rect::Rect, render::Canvas, video::Window,
};

mod bong;

enum Element {
    Div(Vec<Element>),
    Rectangle(Color),
}

impl Element {
    fn render(&self, canvas: &mut Canvas<Window>, x: i32, y: i32) {
        match self {
            Element::Div(_) => todo!(),
            Element::Rectangle(color) => {
                canvas.set_draw_color(*color);
                let _ = canvas.fill_rect(Rect::new(x, y, 50, 50));
            }
        }
    }
}

fn main() {
    let sdl_context = sdl2::init().unwrap();
    let video_subsystem = sdl_context.video().unwrap();

    let window = video_subsystem
        .window("rust-sdl2 demo", 800, 600)
        .position_centered()
        .build()
        .unwrap();

    let mut canvas = window.into_canvas().build().unwrap();

    canvas.set_draw_color(Color::RGB(0, 255, 255));
    canvas.clear();
    canvas.present();
    let mut event_pump = sdl_context.event_pump().unwrap();
    'game_loop: loop {
        canvas.set_draw_color(Color::RGB(170, 200, 255));
        canvas.clear();
        for event in event_pump.poll_iter() {
            match event {
                Event::Quit { .. }
                | Event::KeyDown {
                    keycode: Some(Keycode::Escape),
                    ..
                } => break 'game_loop,
                _ => {}
            }
        }
        // The rest of the game loop goes here...
        let dom = Element::Div(vec![
            Element::Rectangle(Color::RGB(1, 238, 34)),
            Element::Rectangle(Color::RGB(123, 123, 123)),
        ]);

        canvas.set_draw_color(Color::RGB(255, 0, 0));
        let _ = canvas.fill_rect(Rect::new(0, 0, 50, 50));

        canvas.present();
        ::std::thread::sleep(Duration::new(0, 1_000_000_000u32 / 60));
    }
}
