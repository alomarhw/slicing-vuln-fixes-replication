static void udp_v4_rehash(struct sock *sk)
{
	u16 new_hash = udp4_portaddr_hash(sock_net(sk),
					  inet_sk(sk)->inet_rcv_saddr,
					  inet_sk(sk)->inet_num);
	udp_lib_rehash(sk, new_hash);
}
