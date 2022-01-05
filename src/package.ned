package Project;

@license(LGPL);
//
// TODO documentation
//
network Network
{
    parameters:
        double delay = default(0.2);
        double WindowDelay = default(0.05);
        int WindowSize = default(2);
        @display("bgb=650,512");
    types:
    submodules:
        coordinator: Coordinator {
            @display("p=316,244");
        }
        node0: Node {
            @display("p=168,103");
        }
        node1: Node {
            @display("p=439.63202,123.840004");
        }
        node3: Node {
            @display("p=315.792,409.704");
        }
        node2: Node {
            @display("p=135.192,376.68");
        }
        node5: Node {
            @display("p=534.576,224.976");
        }
        node4: Node {
            @display("p=494.328,348.816");
        }
    connections:
        coordinator.out0 --> node0.in_Coordinates;
        coordinator.out1 --> node1.in_Coordinates;
        node1.out --> node0.in;
        node0.out --> node1.in;

        coordinator.out5 --> node5.in_Coordinates;
        coordinator.out4 --> node4.in_Coordinates;
        coordinator.out3 --> node3.in_Coordinates;
        coordinator.out2 --> node2.in_Coordinates;
        node2.out --> node3.in;
        node3.out --> node2.in;
        node4.out --> node5.in;
        node5.out --> node4.in;
}

