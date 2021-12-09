package Project;

@license(LGPL);
//
// TODO documentation
//
network Network
{
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
            @display("p=512,103");
        }
    connections:
        node1.out --> node0.in;
        node0.out --> node1.in;
        coordinator.out1 --> node1.in_Coordinates;
        coordinator.out0 --> node0.in_Coordinates;
}

