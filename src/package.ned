package Project;

@license(LGPL);
//
// TODO documentation
//
network Network
{
    parameters:
        double delay = default(0.2);
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
    connections:
        coordinator.out0 --> node0.in_Coordinates;
        coordinator.out1 --> node1.in_Coordinates;
        node1.out --> node0.in;
        node0.out --> node1.in;
    
}

