proc demo {argument1 {default "DefaultValue"} } {
    puts "This is a demo proc.  It is being called with $argument1 and $default"
    #
    # We can use [info level] to find out if a value was given for
    # the optional argument "default" ...
    #
    puts "Actual call: [info level [info level]]"
}

puts "The args for demo are: [info args demo]\n"
puts "The body for demo is:  [info body demo]\n"

set arglist [info args demo]

foreach arg $arglist {
    if {[info default demo $arg defaultval]} {
        puts "$arg has a default value of $defaultval"
    } else {
        puts "$arg has no default"
    }
}
