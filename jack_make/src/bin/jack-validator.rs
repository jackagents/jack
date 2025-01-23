extern crate jack_make;

fn main() -> Result<(), Box<dyn std::error::Error>>
{
    jack_make::main_validate()
}

