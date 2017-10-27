package main

import "fmt"



func CountChange(money int, coins []int, sol []int) (int) {
	if money == 0 {
		fmt.Println(sol)
		return 1
	} else if len(coins) == 0 || money < 0 {
		return 0
	} else {
		s0 := CountChange(money, coins[1:], sol)
		sol = append(sol, coins[0])
		s1 := CountChange(money - coins[0], coins, sol)
		return s0 + s1
	}
}

func main() {
	coins := []int{1, 2, 5, 10, 20, 50, 100}
	count := CountChange(19, coins, nil)
	fmt.Println(count)
}
