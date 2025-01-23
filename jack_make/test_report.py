#!/usr/bin/python


import json

import sys

tests = { \
            1  : [x for x in range(1, 12)], \
            2  : [x for x in range(1, 16)], \
            3  : [x for x in range(1, 12)], \
            4  : [x for x in range(1, 3)],  \
            5  : [x for x in range(1, 5)],  \
            6  : [0], \
            7  : [0], \
            8  : [x for x in range(1, 12)], \
            9  : [x for x in range(1, 12)], \
            10 : [x for x in range(1,2)]
        }

tested = {}
missed = {}
#print(json.dumps(tests, indent = 4))

def extract_test_data(name):
    test_name, number = name.split("__")
    if len(number.split('_')) == 2:
        big, small = number.split('_')[0:2]

        return [test_name, int(big), int(small)]
    else:
        big_start, small_start, to, big_end, small_end  = number.split('_')
        return [test_name, int(big_start), int(small_start), int(big_end), int(small_end)]
        
with open(sys.argv[1], 'r') as json_file:
    for line in json_file.readlines():
        data = json.loads(line)

        if data['type'] == "test" and data['event'] != "started":
            #print(data['name'], data['event'])
            if len(extract_test_data(data['name'])) == 3:
                name, big, small = extract_test_data(data['name'])
 
                if big in tested:
                        if small in tested[big]:
                            tested[big][small].append({"name" : name, "state": data["event"]})
                        else:
                            tested[big][small] = [{"name" : name, "state": data["event"]}]
                else:
                    tested[big] = { small : [{"name" : name, "state": data["event"]}]}
            else:
                name, big_start, small_start, big_end, small_end = extract_test_data(data['name'])

                for i in range(big_start, big_end + 1):
                    for j in range(small_start, small_end + 1):
                        if i in tested:
                            if j in tested[i]:
                                tested[i][j].append( {"name" : name, "state": data["event"]})
                            else:
                                tested[i][j] = [{"name" : name, "state": data["event"]}]
                        else:
                            tested[i] = { j : [{"name" : name, "state": data["event"]}]}


#    print(json.dumps(tested, indent = 4))
    for k1,v1 in tests.items():
        if k1 in tested:
            for k2 in v1:
                if not k2 in tested[k1]:
                    if k1 in missed:
                        missed[k1].append(k2)
                    else:
                        missed[k1] = [k2]
        else:
            missed[k1] = v1

    out  = "# Test Report for JACK-Validator\n"
    out += "Version: \n" #TODO: Get version
    out += "Tested on:  \n" #TODO: Insert date + time
    out += "\n\n"

    for k1,v1 in tests.items():
        out += "## Tests for Rule #" + str(k1) + "\n"
        if not k1 in tested:
            for k2 in v1:
                out += "Test #"+ str(k1) + "." + str(k2) + " is missing from tests.\n"
        else:
            for k2 in v1:
                if k2 in tested[k1]:
                    passed = False
                    for test in tested[k1][k2]:
                        if test["state"] == "ok":
                            passed |= True
                        else:
                            passed = False
                            break

                    if passed:
                        out += "Rule #"+ str(k1) + "." + str(k2) + " has passed.\n"
                    else:
                        out += "Rule #"+ str(k1) + "." + str(k2) + " has failed.\n"

                else:
                    out += "Rule #"+ str(k1) + "." + str(k2) + " is missing from tests.\n"
        out += "\n"

    print(out)
