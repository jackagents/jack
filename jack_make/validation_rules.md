# Model Validation

## Project
Fields: name and namespace
Project must have a name that is a string that is not empty
Namespace is a list of strings, it can be empty or have one or more strings encapsulated.

## Teams and Agents
Fields: name, action_handlers, beliefsets, goals, initial_goals, plans, resources, roles, message_handlers

Must have a name that is a unique non-zero string
Can have zero or more length list of non-zero length strings in action_handlers. The string must correspond to a message (in messages) of protocol "action" with the same name.
Can have zero or more length list of non-zero length strings in beliefsets. The string must correspond to a message (in messages) of protocol "beliefset" with the same name.
Can have zero or more length list of non-zero length strings in message_handlers. The string must correspond to a message (in messages) of protocol "adhoc" with the same name.
Can have zero or more length list of non-zero length strings in goals. The string must correspond to a goal (in goals) with the same name.
Can have zero or more length list of non-zero length strings in initial_goals. The string must correspond to a goal (in goals) with the same name.
Can have zero or more length list of non-zero length strings in plans. The string must correspond to a plan (in plans) with the same name.
Can have zero or more length list of non-zero length strings in resources. The string must correspond to a resource (in resources) with the same name.
Can have zero or more length list of non-zero length strings in roles. The string must correspond to a resource (in roles) with the same name.

Actions that are in a plan for an agent or team must have a corresponding action_handler field.
Beliefsets must be in roles, plans initial_goals and goals if in an agent or team




## Messages

Fields: name protocol fields
Must have a name that is a unique non-zero string.
protocol must be one of "action", "beliefset" or "adhoc"

### fields 
Fields: "name", "type", "default", "is_array"
At miniimum must contain "name" and "type"

name must be unique for each message
type must be one of "Bool", "Int16", "Int32", "Int64", "Float", "Double", "String" or another message name (must be different to the current message name)
default is a value of <type> (see above)
is_array must be a Bool

## Goals
Fields: "name", "precondition", "dropcondition", "satisfied", "heuristic", "beliefsets", "resources"
Name must be unique
precondition, dropcondition, satisfied and heuristic are all bools

Can have zero or more length list of non-zero length strings in beliefsets. The string must correspond to a message (in messages) of protocol "beliefset" with the same name.

Can have zero or more length list of non-zero length strings in resources. The string must correspond to a resource (in resources) with the same name.

## Plans
Fields: "name", "precondition", "dropcondition", "satisfied", "heuristic", "beliefsets", "tasks", "body"
Name must be unique
precondition, dropcondition, satisfied and heuristic are all bools

Can have zero or more length list of non-zero length strings in beliefsets. The string must correspond to a message (in messages) of protocol "beliefset" with the same name.

Can have zero or more length list of non-zero length strings in body. The string must correspond to a tasks (in this plan's tasks field) with the same name.
tasks are to be merged into body and still under design. This porton will be finishd then.

## Roles
Fields: "name", "goals", "beliefsets"
Name must be unique

Can have zero or more length list of non-zero length strings in goals. The string must correspond to a goal (in goals) with the same name.
Can have zero or more length list of non-zero length objects in beliefsets with fields "name", "read" and "write". The name field must correspond to a message (in messages) of protocol "beliefset" with the same name.

Goals listed in the goal field must have beliefsets with the same name listed in the beliefsets field.

## Resources
Fields: "name", "type", "min", "max"

name must be a unique field




