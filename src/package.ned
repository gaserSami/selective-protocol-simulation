package selective_protocol_simulation;

@license(LGPL);

network Network
{
    @display("bgb=813,299");
    submodules:
        Node0: Node {
            @display("p=131,126");
        }
        Node1: Node {
            @display("p=644,136");
        }
        coordinator: Coordinator {
            @display("p=388,222");
        }
        connections:
            Node0.gout++ --> Node1.gin++;
            Node0.gout++ -->  coordinator.gin++;
            Node1.gout++ --> Node0.gin++;
            Node1.gout++ -->  coordinator.gin++;
            coordinator.gout++ --> Node0.gin++;
            coordinator.gout++ --> Node1.gin++;
}

