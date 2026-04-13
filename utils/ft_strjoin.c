/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_strjoin.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lvitorin <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/17 16:20:07 by lvitorin          #+#    #+#             */
/*   Updated: 2025/06/17 17:35:14 by lvitorin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../Lexer/token.h"

int ft_strlen(const char *s)
{
	if (!s)
		return 0;
	int len = 0;
	while (s[len])
		len++;
	return len;
}

char	*ft_strjoin(const char *s1, const char *s2)
{
	char	*pt;
	int		i;
	int		j;

	if (!s1 || !s2)
		return (NULL);
	pt = (char *)malloc((ft_strlen(s1) + ft_strlen(s2) + 1) * sizeof(char));
	if (!pt)
		return (NULL);
	i = 0;
	while (s1[i])
	{
		pt[i] = s1[i];
		i++;
	}
	j = 0;
	while (s2[j])
	{
		pt[i + j] = s2[j];
		j++;
	}
	pt[i + j] = '\0';
	return (pt);
}
