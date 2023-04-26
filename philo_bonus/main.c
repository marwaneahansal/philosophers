/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mahansal <mahansal@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/03/01 23:22:54 by mahansal          #+#    #+#             */
/*   Updated: 2023/04/26 12:57:42 by mahansal         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo_bonus.h"

void	print_state(t_philo *philo, char *state, int philo_id)
{
	sem_wait(philo->data->state);
	printf("%ld \t %d \t %s\n",
		get_ms_time() - philo->data->start_time, philo_id, state);
	sem_post(philo->data->state);
}

void	sleep_time(long start_time, long end_time)
{
	while (get_ms_time() - start_time < end_time)
		usleep(500);
}

int	check_eat_time(t_data *data, int i)
{
	sem_wait(data->nb_eat);
	if (data->nb_times_of_eating != -1
		&& data->philos[i].eat_count == data->nb_times_of_eating)
	{
		sem_post(data->nb_eat);
		
		return (1);
	}
	sem_post(data->nb_eat);
	return (0);
}

void	*check_dying_philos(void *arg)
{
	t_philo	*philo;
	int	i;

	i = 0;
	philo = (t_philo *) arg;
	while (1)
	{
		if (check_eat_time(philo->data, i))
			exit(0);
		sem_wait(philo->data->last_eat);
		if (get_ms_time() - philo->last_eat_time > philo->data->time_to_die)
		{
			sem_wait(philo->data->state);
			printf("%ld \t %d \t %s\n",
				get_ms_time() - philo->data->start_time,
				philo->data->philos[i].id, "has died");
			philo->data->is_philo_dead = 1;
			sem_post(philo->data->last_eat);
			exit(0);
		}
		sem_post(philo->data->last_eat);
	}
	return (NULL);
}

void	routine(t_philo *philo)
{
	pthread_create(&philo->data->state_routine, NULL, &check_dying_philos, philo);
	if(philo->id % 2 ==0)
		usleep(1000);
	while (1)
	{
		sem_wait(philo->data->forks);
		print_state(philo, "has taken a fork", philo->id);
		sem_wait(philo->data->forks);
		print_state(philo, "has taken a fork", philo->id);
		print_state(philo, "is eating", philo->id);
		sleep_time(get_ms_time(), philo->data->time_to_eat);
		sem_wait(philo->data->nb_eat);
		philo->eat_count++;
		sem_post(philo->data->nb_eat);
		sem_wait(philo->data->last_eat);
		philo->last_eat_time = get_ms_time();
		sem_post(philo->data->last_eat);
		sem_post(philo->data->forks);
		sem_post(philo->data->forks);
		print_state(philo, "is sleeping", philo->id);
		sleep_time(get_ms_time(), philo->data->time_to_sleep);
		print_state(philo, "is thinking", philo->id);
	}
	// pthread_join(philo->data->state_routine, NULL);
}

void	init_semaphors(t_data *data)
{
	sem_unlink("forks");
	sem_unlink("state");
	sem_unlink("nb_eat");
	sem_unlink("last_eat");
	data->forks = sem_open("forks", O_CREAT, 0644, data->philos_nb);
	data->state = sem_open("state", O_CREAT, 0644, 1);
	data->nb_eat = sem_open("nb_eat", O_CREAT, 0644, 1);
	data->last_eat = sem_open("last_eat", O_CREAT, 0644, 1);
}

int	main(int argc, char **argv)
{
	t_data	*data;
	int			i;

	data = NULL;
	i = 0;
	if (argc != 5 && argc != 6)
		return (printf("Error\n"), 1);
	data = init_philos_data(argc, argv);
	if (!data)
		return (printf("Error\n"), 1);
	if (create_philos(data))
		return (1);
	init_semaphors(data);
	while (i < data->philos_nb)
	{
		data->philos[i].process_id = fork();
		if (data->philos[i].process_id == 0)
		{
			routine(&data->philos[i]);
			exit(0);
		}
		i++;
	}
	sem_close(data->forks);
	sem_close(data->state);
	sem_close(data->nb_eat);
	sem_close(data->last_eat);
	waitpid(-1, NULL, 0);
	return (0);
}