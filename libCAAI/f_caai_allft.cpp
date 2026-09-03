/*=========================================================================
//程序名称:     f_caai_allft
//隶属子系统:   CA
//产品名称:     BM2PES
//创建人员:     ZHOULI
//创建时间:     2012-11-26
//修改人员:     
//修改日期:  
//函数说明：需要分摊的数量或金额进行分摊。RULE_TYPE = '2' 分摊重量，RULE_TYPE = '3'分摊金额，RULE_TYPE = '4' 能源特殊
//=========================================================================*/
//框架公用头文件，勿删
#include "stdafx.h"
//程序用头文件

BM2_FUNCTION_EXPORT
int f_caai_updprice(CString stats_period, CString dept_code, CString type, CDbConnection * conn);
int f_caai_allft(CString account_period, CString dept_code, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	int i = 0;
	int doFlag = 0;	

	CString sqlstr = "";
	CString datenow = CDateTime::Now().ToString("yyyyMMddHHmmss");	
	CDecimal total_wt, dif_wt;
	CString sqlstr_sub = "";
	CDecimal price = 0;
	CDecimal price2 = 0;
	CString divvy_type = "";
	CString begin_date = "";
	CString end_date = "";



	// 创建电文处理对象
	CDbCommand cmd_inq(conn);
	CDbCommand cmd_inq_1(conn);
	CDbCommand cmd_inq_sub(conn);

	try
	{	
		//取会计期的开始结束时间
		sqlstr = " select S_DATETIME,E_DATETIME from tcaac15"
			" where account_period = @account_period"
			;
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("account_period", account_period);
		cmd_inq.ExecuteReader();
		while (cmd_inq.Read())
		{
			begin_date = cmd_inq.GetString(1);
			end_date = cmd_inq.GetString(2);
		}
		cmd_inq.Close();


		//产出
		sqlstr = " delete from tcaai05"
			" where 1=1"
			" AND dept_code = @dept_code"
			" and account_period = @account_period"
			;
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("dept_code", dept_code);
		cmd_inq.Parameters.Set("account_period", account_period);
		cmd_inq.ExecuteNonQuery();
		cmd_inq.Close();

				
		//插入本月的数量消耗
		sqlstr = " insert into tcaai05(rec_creator, rec_create_time,dept_code,account_period, cost_center, sub_backlog_code, equ_no, sg_sign, mat_thick, mat_width, product_code,PRODUCT_CODE_CNAME,  vfree1, vfree2, vfree3, vfree4, vfree5, qty, wt,PROD_DATE )"
			" select @rec_creator, @rec_create_time,dept_code,@account_period,cost_center, sub_backlog_code, equ_no, sg_sign, mat_thick, mat_width, product_code,PRODUCT_CODE_CNAME,  vfree1, vfree2, vfree3, vfree4, vfree5, sum(qty), sum(wt),STATS_PERIOD"
			" from tcaai03"
			" where dept_code = @dept_code"
			" and stats_period<=@end_date"
			" and stats_period>=@begin_date"
			" group by dept_code,cost_center, sub_backlog_code, equ_no, sg_sign, mat_thick, mat_width, product_code,PRODUCT_CODE_CNAME,  vfree1, vfree2, vfree3, vfree4, vfree5,STATS_PERIOD"
			;
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("dept_code", dept_code);
		cmd_inq.Parameters.Set("account_period", account_period);
		cmd_inq.Parameters.Set("begin_date", begin_date);
		cmd_inq.Parameters.Set("end_date", end_date);
		cmd_inq.Parameters.Set("rec_creator", s.userid);
		cmd_inq.Parameters.Set("rec_create_time", datenow);
		cmd_inq.ExecuteNonQuery();
		cmd_inq.Close();

		sqlstr = " delete from tcaai06"
			" where 1=1"
			" AND dept_code = @dept_code"
			" and account_period = @account_period"
			;
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("dept_code", dept_code);
		cmd_inq.Parameters.Set("account_period", account_period);
		cmd_inq.ExecuteNonQuery();
		cmd_inq.Close();


		//插入本月消耗
		sqlstr = " insert into tcaai06 (rec_creator,rec_create_time,account_period,dept_code,cost_center,sub_backlog_code,PRODUCT_CODE,PRODUCT_CODE_CNAME,MAT_CODE,PROD_DATE"
			" , equ_no, sg_sign, mat_thick, mat_width, vfree1, vfree2, vfree3, vfree4, vfree5,pass_wt,WT )"
			" select @rec_creator, @rec_create_time,@account_period,dept_code,cost_center, sub_backlog_code,PRODUCT_CODE,PRODUCT_CODE_CNAME,MAT_CODE,STATS_PERIOD"
			",equ_no, sg_sign, mat_thick, mat_width, vfree1, vfree2, vfree3, vfree4, vfree5, sum(WT), sum(WT)"
			" from tcaai04"
			" where 1=1"
			" and vfree1!=' '"
			" and dept_code = @dept_code"
			" and stats_period<=@end_date"
			" and stats_period>=@begin_date"
			" group by dept_code,cost_center, sub_backlog_code,  product_code,PRODUCT_CODE_CNAME,MAT_CODE,equ_no, sg_sign, mat_thick, mat_width,  vfree1, vfree2, vfree3, vfree4, vfree5,STATS_PERIOD"
			" having sum(WT)!=0"
			;
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("dept_code", dept_code);
		cmd_inq.Parameters.Set("account_period", account_period);
		cmd_inq.Parameters.Set("begin_date", begin_date);
		cmd_inq.Parameters.Set("end_date", end_date);
		cmd_inq.Parameters.Set("rec_creator", s.userid);
		cmd_inq.Parameters.Set("rec_create_time", datenow);
		cmd_inq.ExecuteNonQuery();
		cmd_inq.Close();

		//副产品
	    //更新最新的量为0，以防数据清0
		sqlstr = " update tcaai06 set wt = 0"
			" where 1=1"
			" and mat_code in (select mat_code from tcaac11 where mat_type in ('A','D'))"
			" and dept_code = @dept_code"
			" and account_period = @account_period"
			;
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("dept_code", dept_code);
		cmd_inq.Parameters.Set("account_period", account_period);
		cmd_inq.ExecuteNonQuery();
		cmd_inq.Close();

		sqlstr = " select sub_backlog_code,mat_code,0-sum(NET_WT) all_use"
			" from ("
			" select DST_MAT_CODE as mat_code,SRC_STOCK_CODE ,NET_WT,NVL(BACK_C2,' ') sub_Backlog_code"
			" from tymrec1fg t1"
			" left join tcaaib1 t2 on t1.SRC_STOCK_CODE = t2.BACK_C1 AND t2.BACK_C3 = @dept_code AND t2.PROJECT_ID = 'HS'"
			" where WEIGH_APP_TYPE = 'HF' "
			" AND substr(WEIGH_APP_TIME,1,8) <= @end_date"
			" AND substr(WEIGH_APP_TIME,1,8) >= @begin_date"
			" )"
			" where sub_Backlog_code !=' '"
			" having sum(NET_WT)!=0"
			" group by sub_backlog_code,mat_code"
			;
		/*sqlstr = "select sub_backlog_code,mat_code,0-sum(NET_WT) all_use"
			" from tcaais6"
			" where dept_code = @dept_code"
			" and substr(WEIGH_TIME,1,8)<=@end_date"
			" and substr(WEIGH_TIME,1,8)>=@begin_date"
			" group by sub_backlog_code,mat_code"
			" having sum(NET_WT)!=0"
			;
			*/
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("dept_code", dept_code);
		cmd_inq.Parameters.Set("begin_date", begin_date);
		cmd_inq.Parameters.Set("end_date", end_date);
		cmd_inq.Parameters.Set("account_period", account_period);
		cmd_inq.ExecuteReader();
		while (cmd_inq.Read())
		{
			sqlstr = "select sum(pass_wt) total_wt from tcaai06"
				" where 1=1"
				" AND pass_wt<0"
				" and mat_code = @mat_code"
				" and sub_backlog_code = @sub_backlog_code"
				" and dept_code = @dept_code"
				" and account_period = @account_period"
				;
			cmd_inq_1.SetCommandText(sqlstr);
			cmd_inq_1.Parameters.Set("dept_code", dept_code);
			cmd_inq_1.Parameters.Set("account_period", account_period);
			cmd_inq_1.Parameters.Set("sub_backlog_code", cmd_inq.GetString(1));
			cmd_inq_1.Parameters.Set("mat_code", cmd_inq.GetString(2));
			cmd_inq_1.ExecuteReader();
			total_wt = 0;
			if (cmd_inq_1.Read())
			{
				total_wt = cmd_inq_1.GetDecimal(1);
			}
			cmd_inq_1.Close();

			if (total_wt != 0)
			{
				//更新数据
				sqlstr = " update tcaai06 t set wt = round(pass_wt*@all_use/@total_wt,3)"
					" where 1=1"
					" AND pass_wt<0"
					" and mat_code = @mat_code"
					" and sub_backlog_code = @sub_backlog_code"
					" and dept_code = @dept_code"
					" and account_period = @account_period"
					;
				cmd_inq_1.SetCommandText(sqlstr);
				cmd_inq_1.Parameters.Set("dept_code", dept_code);
				cmd_inq_1.Parameters.Set("account_period", account_period);
				cmd_inq_1.Parameters.Set("sub_backlog_code", cmd_inq.GetString(1));
				cmd_inq_1.Parameters.Set("mat_code", cmd_inq.GetString(2));
				cmd_inq_1.Parameters.Set("all_use", cmd_inq.GetDecimal(3));
				cmd_inq_1.Parameters.Set("total_wt", total_wt);
				cmd_inq_1.ExecuteNonQuery();
				cmd_inq_1.Close();				
			}
			else  //如果原来没有消耗，则按分摊方式进行分摊
			{
				//分摊方式
				divvy_type = "1";				
				sqlstr = "select MEASURE_MODE"
					" from tcaac07 "
					" where mat_code = @mat_code"
					" and sub_backlog_code = @sub_backlog_code"
					;

				cmd_inq_1.SetCommandText(sqlstr);
				cmd_inq_1.Parameters.Set("sub_backlog_code", cmd_inq.GetString(1));
				cmd_inq_1.Parameters.Set("mat_code", cmd_inq.GetString(2));
				cmd_inq_1.ExecuteReader();
				if (cmd_inq_1.Read())
				{
					divvy_type = cmd_inq_1.GetString(1);
				}
				cmd_inq_1.Close();

				sqlstr = "select sum(DIVVY_BASIC_N) total_wt from tcaai01"
					" where 1=1"
					" and sub_backlog_code = @sub_backlog_code"
					" and dept_code = @dept_code"
					" and stats_period<=@end_date"
					" and stats_period>=@begin_date"
					;
				cmd_inq_1.SetCommandText(sqlstr);
				cmd_inq_1.Parameters.Set("dept_code", dept_code);
				cmd_inq_1.Parameters.Set("account_period", account_period);
				cmd_inq_1.Parameters.Set("end_date", end_date);
				cmd_inq_1.Parameters.Set("begin_date", begin_date);
				cmd_inq_1.Parameters.Set("sub_backlog_code", cmd_inq.GetString(1));
				cmd_inq_1.ExecuteReader();
				total_wt = 0;
				if (cmd_inq_1.Read())
				{
					total_wt = cmd_inq_1.GetDecimal(1);
				}
				cmd_inq_1.Close();

				if (total_wt == 0)
				{
					
					strcpy(s.msg, "分摊方式的服务量为0，请更换分摊方式!");
					throw CApplicationException(-1, s.msg, log.Location);
				}
				else
				{
					sqlstr = " insert into tcaai06 (REC_CREATE_TIME,dept_code,account_period,sub_backlog_code,PROD_DATE,cost_center,product_CODE,equ_no, sg_sign, mat_thick, mat_width,MAT_CODE,vfree1,vfree2,vfree3,vfree4,vfree5,WT)"
						" select @datenow,dept_code,@account_period,sub_backlog_code,stats_period,cost_center,product_CODE,equ_no, sg_sign, mat_thick, mat_width,@mat_code,vfree1,vfree2,vfree3,vfree4,vfree5,round(SUM(DIVVY_BASIC_N)*@all_use/@total_wt,3) "
						" from tcaai01"
						" where 1=1"
						" and sub_backlog_code = @sub_backlog_code"
						" and dept_code = @dept_code"
						" and stats_period<=@end_date"
						" and stats_period>=@begin_date"
						" group by dept_code,sub_backlog_code,stats_period,cost_center,product_CODE,equ_no, sg_sign, mat_thick, mat_width,vfree1,vfree2,vfree3,vfree4,vfree5"
						;
					cmd_inq_1.SetCommandText(sqlstr);
					cmd_inq_1.Parameters.Set("datenow", datenow);
					cmd_inq_1.Parameters.Set("dept_code", dept_code);
					cmd_inq_1.Parameters.Set("account_period", account_period);
					cmd_inq_1.Parameters.Set("end_date", end_date);
					cmd_inq_1.Parameters.Set("begin_date", begin_date);
					cmd_inq_1.Parameters.Set("sub_backlog_code", cmd_inq.GetString(1));
					cmd_inq_1.Parameters.Set("mat_code", cmd_inq.GetString(2));
					cmd_inq_1.Parameters.Set("all_use", cmd_inq.GetDecimal(3));
					cmd_inq_1.Parameters.Set("total_wt", total_wt);
					cmd_inq_1.ExecuteNonQuery();
					cmd_inq_1.Close();
				}
			}

			sqlstr = "select sum(wt)  from tcaai06"
				" where 1=1"
				" and mat_code = @mat_code"
				" and sub_backlog_code = @sub_backlog_code"
				" and dept_code = @dept_code"
				" and account_period = @account_period"
				;
			cmd_inq_1.SetCommandText(sqlstr);
			cmd_inq_1.Parameters.Set("dept_code", dept_code);
			cmd_inq_1.Parameters.Set("account_period", account_period);
			cmd_inq_1.Parameters.Set("sub_backlog_code", cmd_inq.GetString(1));
			cmd_inq_1.Parameters.Set("mat_code", cmd_inq.GetString(2));
			cmd_inq_1.ExecuteReader();
			dif_wt = 0;
			if (cmd_inq_1.Read())
			{
				dif_wt = cmd_inq.GetDecimal(3) - cmd_inq_1.GetDecimal(1);
			}
			cmd_inq_1.Close();

			//更新尾差
			if (dif_wt != 0)
			{
				sqlstr = "update tcaai06 set wt = wt + @dif_wt"
					" where 1=1"
					" and wt in (select max(wt) from tcaai06 where  mat_code = @mat_code and sub_backlog_code = @sub_backlog_code and dept_code = @dept_code and account_period = @account_period)"
					" and mat_code = @mat_code"
					" and sub_backlog_code = @sub_backlog_code"
					" and dept_code = @dept_code"
					" and account_period = @account_period"
					" and rownum = 1"
					" order by vfree1"
					;
				cmd_inq_1.SetCommandText(sqlstr);
				cmd_inq_1.Parameters.Set("dept_code", dept_code);
				cmd_inq_1.Parameters.Set("account_period", account_period);
				cmd_inq_1.Parameters.Set("sub_backlog_code", cmd_inq.GetString(1));
				cmd_inq_1.Parameters.Set("mat_code", cmd_inq.GetString(2));
				cmd_inq_1.Parameters.Set("dif_wt", dif_wt);
				cmd_inq_1.ExecuteNonQuery();
				cmd_inq_1.Close();
			}
		}
		cmd_inq.Close();


		//能源
		sqlstr = "select mat_code,sum(COMSUME_WT) all_use,equ_no"
			" from tcaaia12"
			" where busi_type = 'YJ'"
			" and equ_no !='ZH'"
			" and dept_code = @dept_code"
			" and prod_date=@account_period"
			" group by mat_code,equ_no"
			" HAVING sum(COMSUME_WT)!=0"
			;
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("account_period", account_period);
		cmd_inq.Parameters.Set("dept_code", dept_code);
		cmd_inq.ExecuteReader();
		while (cmd_inq.Read())
		{
			sqlstr = "select sum(pass_wt) total_wt from tcaai06"
				" where 1=1"
				" and equ_no =@equ_no"
				" and mat_code = @mat_code"
				" and dept_code = @dept_code"
				" and account_period = @account_period"
				;
			cmd_inq_1.SetCommandText(sqlstr);
			cmd_inq_1.Parameters.Set("dept_code", dept_code);
			cmd_inq_1.Parameters.Set("account_period", account_period);
			cmd_inq_1.Parameters.Set("mat_code", cmd_inq.GetString(1));
			cmd_inq_1.Parameters.Set("equ_no", cmd_inq.GetString(3));
			cmd_inq_1.ExecuteReader();
			total_wt = 0;
			if (cmd_inq_1.Read())
			{
				total_wt = cmd_inq_1.GetDecimal(1);
			}
			cmd_inq_1.Close();

			if (total_wt != 0)
			{
				//更新数据
				sqlstr = " update tcaai06 t set wt = round(pass_wt*@all_use/@total_wt,3)"
					" where 1=1"
					" and equ_no =@equ_no"
					" and mat_code = @mat_code"
					" and dept_code = @dept_code"
					" and account_period = @account_period"
					;
				cmd_inq_1.SetCommandText(sqlstr);
				cmd_inq_1.Parameters.Set("dept_code", dept_code);
				cmd_inq_1.Parameters.Set("account_period", account_period);
				cmd_inq_1.Parameters.Set("mat_code", cmd_inq.GetString(1));
				cmd_inq_1.Parameters.Set("all_use", cmd_inq.GetDecimal(2));
				cmd_inq_1.Parameters.Set("equ_no", cmd_inq.GetString(3));
				cmd_inq_1.Parameters.Set("total_wt", total_wt);
				cmd_inq_1.ExecuteNonQuery();
				cmd_inq_1.Close();
			}
			else  //如果原来没有消耗，则按分摊方式进行分摊
			{
				//分摊方式
				divvy_type = "1";
				sqlstr = "select MEASURE_MODE"
					" from tcaac07 "
					" where mat_code = @mat_code"
					;

				cmd_inq_1.SetCommandText(sqlstr);
				cmd_inq_1.Parameters.Set("sub_backlog_code", cmd_inq.GetString(1));
				cmd_inq_1.Parameters.Set("mat_code", cmd_inq.GetString(2));
				cmd_inq_1.ExecuteReader();
				if (cmd_inq_1.Read())
				{
					divvy_type = cmd_inq_1.GetString(1);
				}
				cmd_inq_1.Close();

				sqlstr = "select sum(DIVVY_BASIC_N) total_wt from tcaai01"
					" where 1=1"
					" and equ_no =@equ_no"
					" and divvy_type =@divvy_type"
					" and dept_code = @dept_code"
					" and stats_period<=@end_date"
					" and stats_period>=@begin_date"
					;
				cmd_inq_1.SetCommandText(sqlstr);
				cmd_inq_1.Parameters.Set("dept_code", dept_code);
				cmd_inq_1.Parameters.Set("end_date", end_date);
				cmd_inq_1.Parameters.Set("begin_date", begin_date);
				cmd_inq_1.Parameters.Set("divvy_type", divvy_type);
				cmd_inq_1.Parameters.Set("equ_no", cmd_inq.GetString(3));
				cmd_inq_1.ExecuteReader();
				total_wt = 0;
				if (cmd_inq_1.Read())
				{
					total_wt = cmd_inq_1.GetDecimal(1);
				}
				cmd_inq_1.Close();

				if (total_wt == 0)
				{//如果没有生产，则不进行分摊，走费用请示
					/*strcpy(s.msg, "分摊方式的服务量为0，请更换分摊方式!");
					throw CApplicationException(-1, s.msg, log.Location);*/
				}
				else
				{
					sqlstr = " insert into tcaai06 (REC_CREATE_TIME,dept_code,account_period,sub_backlog_code,PROD_DATE,cost_center,product_CODE,equ_no, sg_sign, mat_thick, mat_width,MAT_CODE,vfree1,vfree2,vfree3,vfree4,vfree5,WT,data_from)"
						" select @datenow,dept_code,@account_period,sub_backlog_code,stats_period,cost_center,product_CODE,equ_no, sg_sign, mat_thick, mat_width,@mat_code,vfree1,vfree2,vfree3,vfree4,vfree5,round(SUM(DIVVY_BASIC_N)*@all_use/@total_wt,3),@equ_no "
						" from tcaai01"
						" where 1=1"
						" and equ_no=@equ_no"
						" and dept_code = @dept_code"
						" and stats_period<=@end_date"
						" and stats_period>=@begin_date"
						" group by dept_code,sub_backlog_code,stats_period,cost_center,product_CODE,equ_no, sg_sign, mat_thick, mat_width,vfree1,vfree2,vfree3,vfree4,vfree5"
						;
					cmd_inq_1.SetCommandText(sqlstr);
					cmd_inq_1.Parameters.Set("datenow", datenow);
					cmd_inq_1.Parameters.Set("dept_code", dept_code);
					cmd_inq_1.Parameters.Set("account_period", account_period);
					cmd_inq_1.Parameters.Set("end_date", end_date);
					cmd_inq_1.Parameters.Set("begin_date", begin_date);
					cmd_inq_1.Parameters.Set("mat_code", cmd_inq.GetString(1));
					cmd_inq_1.Parameters.Set("all_use", cmd_inq.GetDecimal(2));
					cmd_inq_1.Parameters.Set("equ_no", cmd_inq.GetString(3));
					cmd_inq_1.Parameters.Set("total_wt", total_wt);
					cmd_inq_1.ExecuteNonQuery();
					cmd_inq_1.Close();
				}
			}

			sqlstr = "select sum(wt)  from tcaai06"
				" where 1=1"
				" and equ_no =@equ_no"
				" and mat_code = @mat_code"
				" and dept_code = @dept_code"
				" and account_period = @account_period"
				;
			cmd_inq_1.SetCommandText(sqlstr);
			cmd_inq_1.Parameters.Set("dept_code", dept_code);
			cmd_inq_1.Parameters.Set("account_period", account_period);
			cmd_inq_1.Parameters.Set("mat_code", cmd_inq.GetString(1));
			cmd_inq_1.Parameters.Set("equ_no", cmd_inq.GetString(3));
			cmd_inq_1.ExecuteReader();
			dif_wt = 0;
			if (cmd_inq_1.Read())
			{
				dif_wt = cmd_inq.GetDecimal(2) - cmd_inq_1.GetDecimal(1);
			}
			cmd_inq_1.Close();

			//更新尾差
			if (dif_wt != 0)
			{
				sqlstr = "update tcaai06 set wt = wt + @dif_wt"
					" where 1=1"
					" and wt in (select max(wt) from tcaai06 where equ_no =@equ_no and mat_code = @mat_code and dept_code = @dept_code and account_period = @account_period)"
					" and equ_no =@equ_no"
					" and mat_code = @mat_code"
					" and dept_code = @dept_code"
					" and account_period = @account_period"
					" and rownum = 1"	
					" order by vfree1"
					;
				cmd_inq_1.SetCommandText(sqlstr);
				cmd_inq_1.Parameters.Set("dept_code", dept_code);
				cmd_inq_1.Parameters.Set("account_period", account_period);
				cmd_inq_1.Parameters.Set("mat_code", cmd_inq.GetString(1));
				cmd_inq_1.Parameters.Set("equ_no", cmd_inq.GetString(3));
				cmd_inq_1.Parameters.Set("dif_wt", dif_wt);
				cmd_inq_1.ExecuteNonQuery();
				cmd_inq_1.Close();
			}
		}
		cmd_inq.Close();

		//综合的能耗
		sqlstr = "select mat_code,sum(COMSUME_WT) all_use"
			" from tcaaia12"
			" where busi_type = 'YJ'"
			" and (equ_no ='ZH' or equ_no not in (select equ_no from tcaai05 where  dept_code = @dept_code and account_period =@account_period ))"
			" and dept_code = @dept_code"
			" and prod_date=@account_period"
			" group by mat_code,equ_no"
			" HAVING sum(COMSUME_WT)!=0"
			;
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("account_period", account_period);
		cmd_inq.Parameters.Set("dept_code", dept_code);
		cmd_inq.ExecuteReader();
		while (cmd_inq.Read())
		{			
				//分摊方式
				divvy_type = "1";
				if (dept_code == "F")
				{
					divvy_type = "3";
				}
				sqlstr = "select MEASURE_MODE"
					" from tcaac07 "
					" where mat_code = @mat_code"
					" AND DEPT_CODE = @dept_code"
					;

				cmd_inq_1.SetCommandText(sqlstr);
				cmd_inq_1.Parameters.Set("sub_backlog_code", cmd_inq.GetString(1));
				cmd_inq_1.Parameters.Set("mat_code", cmd_inq.GetString(2));
				cmd_inq_1.Parameters.Set("dept_code", dept_code);
				cmd_inq_1.ExecuteReader();
				if (cmd_inq_1.Read())
				{
					divvy_type = cmd_inq_1.GetString(1);
				}
				cmd_inq_1.Close();

				sqlstr = "select sum(DIVVY_BASIC_N) total_wt from tcaai01"
					" where 1=1"
					" and DIVVY_BASIC_N > 0"
					" and divvy_type =@divvy_type"
					" and dept_code = @dept_code"
					" and stats_period<=@end_date"
					" and stats_period>=@begin_date"
					;
				cmd_inq_1.SetCommandText(sqlstr);
				cmd_inq_1.Parameters.Set("dept_code", dept_code);
				cmd_inq_1.Parameters.Set("end_date", end_date);
				cmd_inq_1.Parameters.Set("begin_date", begin_date);
				cmd_inq_1.Parameters.Set("divvy_type", divvy_type);
				cmd_inq_1.ExecuteReader();
				total_wt = 0;
				if (cmd_inq_1.Read())
				{
					total_wt = cmd_inq_1.GetDecimal(1);
				}
				cmd_inq_1.Close();

				if (total_wt == 0)
				{
					strcpy(s.msg, "分摊方式的服务量为0，请更换分摊方式!");
					throw CApplicationException(-1, s.msg, log.Location);
				}
				else
				{
					sqlstr = " insert into tcaai06 (REC_CREATE_TIME,dept_code,account_period,sub_backlog_code,prod_Date,cost_center,product_CODE,equ_no, sg_sign, mat_thick, mat_width,MAT_CODE,vfree1,vfree2,vfree3,vfree4,vfree5,WT,data_from)"
						" select @datenow,dept_code,@account_period,sub_backlog_code,stats_period,cost_center,product_CODE,equ_no, sg_sign, mat_thick, mat_width,@mat_code,vfree1,vfree2,vfree3,vfree4,vfree5,round(SUM(DIVVY_BASIC_N)*@all_use/@total_wt,3),'ZH' "
						" from tcaai01"
						" where 1=1"
						" and divvy_type =@divvy_type"
						" and DIVVY_BASIC_N > 0"
						" and dept_code = @dept_code"
						" and stats_period<=@end_date"
						" and stats_period>=@begin_date"
						" group by dept_code,sub_backlog_code,stats_period,cost_center,product_CODE,equ_no, sg_sign, mat_thick, mat_width,vfree1,vfree2,vfree3,vfree4,vfree5"
						;
					cmd_inq_1.SetCommandText(sqlstr);
					cmd_inq_1.Parameters.Set("datenow", datenow);
					cmd_inq_1.Parameters.Set("dept_code", dept_code);
					cmd_inq_1.Parameters.Set("account_period", account_period);
					cmd_inq_1.Parameters.Set("end_date", end_date);
					cmd_inq_1.Parameters.Set("begin_date", begin_date);
					cmd_inq_1.Parameters.Set("mat_code", cmd_inq.GetString(1));
					cmd_inq_1.Parameters.Set("all_use", cmd_inq.GetDecimal(2));
					cmd_inq_1.Parameters.Set("total_wt", total_wt);
					cmd_inq_1.ExecuteNonQuery();
					cmd_inq_1.Close();
			}

			sqlstr = "select sum(wt)  from tcaai06"
				" where 1=1"
				" and DATA_FROM ='ZH'"
				" and mat_code = @mat_code"
				" and dept_code = @dept_code"
				" and account_period = @account_period"
				;
			cmd_inq_1.SetCommandText(sqlstr);
			cmd_inq_1.Parameters.Set("dept_code", dept_code);
			cmd_inq_1.Parameters.Set("account_period", account_period);
			cmd_inq_1.Parameters.Set("mat_code", cmd_inq.GetString(1));
			cmd_inq_1.ExecuteReader();
			dif_wt = 0;
			if (cmd_inq_1.Read())
			{
				dif_wt = cmd_inq.GetDecimal(2) - cmd_inq_1.GetDecimal(1);
			}
			cmd_inq_1.Close();

			//更新尾差
			if (dif_wt != 0)
			{
				sqlstr = "update tcaai06 set wt = wt + @dif_wt"
					" where 1=1"
					" and wt in (select max(wt) from tcaai06 where DATA_FROM ='ZH' and mat_code = @mat_code and dept_code = @dept_code and account_period = @account_period)"
					" and DATA_FROM  ='ZH'"
					" and mat_code = @mat_code"
					" and dept_code = @dept_code"
					" and account_period = @account_period"
					" and rownum = 1"
					" order by vfree1"
					;
				cmd_inq_1.SetCommandText(sqlstr);
				cmd_inq_1.Parameters.Set("dept_code", dept_code);
				cmd_inq_1.Parameters.Set("account_period", account_period);
				cmd_inq_1.Parameters.Set("mat_code", cmd_inq.GetString(1));
				cmd_inq_1.Parameters.Set("dif_wt", dif_wt);
				cmd_inq_1.ExecuteNonQuery();
				cmd_inq_1.Close();
			}
		}
		cmd_inq.Close();

		doFlag = f_caai_updprice(account_period, dept_code," ", conn);
		if (doFlag != 0)
		{
			strcpy(s.msg, "更新价格出错!");
			throw CApplicationException(-1, s.msg, log.Location);
		}

		//费用
		sqlstr = "select cost_center,mat_code,sum(amt) all_use"
			" from TCAAIA11"
			" where 1=1"
			" and cost_center in (select cost_center from tcaac01 where dept_code = @dept_code AND REMARK!= ' ')"
			" and account_period = @account_period"
			" group by cost_center,mat_code"
			;
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("account_period", account_period);
		cmd_inq.Parameters.Set("dept_code", dept_code);
		cmd_inq.ExecuteReader();
		while (cmd_inq.Read())
		{
			sqlstr = "select sum(pass_amt) total_wt from tcaai06 t1"
				" where 1=1"
				" and mat_code = @mat_code"
				" and EXISTS (select 1 FROM TCAAC01 t2 WHERE  instr(t2.REMARK, t1.equ_no)>0 AND t2.COST_CENTER = @cost_center)"
				" and data_From=@cost_center"
				" and dept_code = @dept_code"
				" and account_period = @account_period"
				;
			cmd_inq_1.SetCommandText(sqlstr);
			cmd_inq_1.Parameters.Set("dept_code", dept_code);
			cmd_inq_1.Parameters.Set("account_period", account_period);
			cmd_inq_1.Parameters.Set("cost_center", cmd_inq.GetString(1));
			cmd_inq_1.Parameters.Set("mat_code", cmd_inq.GetString(2));
			cmd_inq_1.ExecuteReader();
			total_wt = 0;
			if (cmd_inq_1.Read())
			{
				total_wt = cmd_inq_1.GetDecimal(1);
			}
			cmd_inq_1.Close();

			if (total_wt != 0)
			{
				//更新数据
				sqlstr = " update tcaai06 t1 set COST_HJ = round(pass_amt*@all_use/@total_wt,3)"
					" where 1=1"
					" and mat_code = @mat_code"
					" and EXISTS (select 1 FROM TCAAC01 t2 WHERE  instr(t2.REMARK, t1.equ_no)>0 AND t2.COST_CENTER = @cost_center)"
					" and dept_code = @dept_code"
					" and account_period = @account_period"
					;
				cmd_inq_1.SetCommandText(sqlstr);
				cmd_inq_1.Parameters.Set("dept_code", dept_code);
				cmd_inq_1.Parameters.Set("account_period", account_period);
				cmd_inq_1.Parameters.Set("cost_center", cmd_inq.GetString(1));
				cmd_inq_1.Parameters.Set("mat_code", cmd_inq.GetString(2));
				cmd_inq_1.Parameters.Set("all_use", cmd_inq.GetDecimal(3));
				cmd_inq_1.Parameters.Set("total_wt", total_wt);
				cmd_inq_1.ExecuteNonQuery();
				cmd_inq_1.Close();
			}
			else  //如果原来没有消耗，则按分摊方式进行分摊
			{
				//按工时分摊方式，如果是废钢则按产量
				divvy_type = "1";
				if (dept_code == "F")
				{
					divvy_type = "3";
				}
				
				
				sqlstr = "select sum(DIVVY_BASIC_N) total_wt from tcaai01 t1"
					" where 1=1"
					" and divvy_type =@divvy_type"
					" and EXISTS (select 1 FROM TCAAC01 t2 WHERE  instr(t2.REMARK, t1.equ_no)>0 AND t2.COST_CENTER = @cost_center and REMARK!=' ')"
					" and dept_code = @dept_code"
					" and stats_period<=@end_date"
					" and stats_period>=@begin_date"
					;
				cmd_inq_1.SetCommandText(sqlstr);
				cmd_inq_1.Parameters.Set("dept_code", dept_code);
				cmd_inq_1.Parameters.Set("end_date", end_date);
				cmd_inq_1.Parameters.Set("begin_date", begin_date);
				cmd_inq_1.Parameters.Set("account_period", account_period);
				cmd_inq_1.Parameters.Set("cost_center", cmd_inq.GetString(1));
				cmd_inq_1.Parameters.Set("divvy_type", divvy_type);
				cmd_inq_1.ExecuteReader();
				total_wt = 0;
				if (cmd_inq_1.Read())
				{
					total_wt = cmd_inq_1.GetDecimal(1);
				}
				cmd_inq_1.Close();

				if (total_wt == 0)
				{
					Log::Trace("", "", "COST_CENTER = [{0}],divvy_type = [{1}],end_date={2},begin_date=[{3}],account_period = [{4}]", cmd_inq.GetString(1), divvy_type, end_date, begin_date, account_period);
					/*strcpy(s.msg, "分摊方式的服务量为0，请更换分摊方式!");
					throw CApplicationException(-1, s.msg, log.Location);*/
				}
				else
				{
					sqlstr = " insert into tcaai06 (REC_CREATE_TIME,dept_code,account_period,sub_backlog_code,prod_date,cost_center,product_CODE,equ_no, sg_sign, mat_thick, mat_width,MAT_CODE,vfree1,vfree2,vfree3,vfree4,vfree5,COST_HJ,data_from)"
						" select @datenow,dept_code,@account_period,sub_backlog_code,stats_period,cost_center,product_CODE,equ_no, sg_sign, mat_thick, mat_width,@mat_code,vfree1,vfree2,vfree3,vfree4,vfree5,round(SUM(DIVVY_BASIC_N)*@all_use/@total_wt,3),@cost_center "
						" from tcaai01 t1"
						" where 1=1"
						" and EXISTS (select 1 FROM TCAAC01 t2 WHERE  instr(t2.REMARK, t1.equ_no)>0 AND t2.COST_CENTER = @cost_center)"
						" and dept_code = @dept_code"
						" and stats_period<=@end_date"
						" and stats_period>=@begin_date"
						" group by dept_code,sub_backlog_code,stats_period,cost_center,product_CODE,equ_no, sg_sign, mat_thick, mat_width,vfree1,vfree2,vfree3,vfree4,vfree5"
						;
					cmd_inq_1.SetCommandText(sqlstr);
					cmd_inq_1.Parameters.Set("datenow", datenow);
					cmd_inq_1.Parameters.Set("dept_code", dept_code);
					cmd_inq_1.Parameters.Set("end_date", end_date);
					cmd_inq_1.Parameters.Set("begin_date", begin_date);
					cmd_inq_1.Parameters.Set("account_period", account_period);
					cmd_inq_1.Parameters.Set("cost_center", cmd_inq.GetString(1));
					cmd_inq_1.Parameters.Set("mat_code", cmd_inq.GetString(2));
					cmd_inq_1.Parameters.Set("all_use", cmd_inq.GetDecimal(3));
					cmd_inq_1.Parameters.Set("total_wt", total_wt);
					cmd_inq_1.ExecuteNonQuery();
					cmd_inq_1.Close();
				}
			}

			sqlstr = "select sum(COST_HJ)  from tcaai06 t1"
				" where 1=1"
				" and data_from =@cost_center"
				" and mat_code = @mat_code"				
				" and dept_code = @dept_code"
				" and account_period = @account_period"
				;
			cmd_inq_1.SetCommandText(sqlstr);
			cmd_inq_1.Parameters.Set("dept_code", dept_code);
			cmd_inq_1.Parameters.Set("account_period", account_period);
			cmd_inq_1.Parameters.Set("cost_center", cmd_inq.GetString(1));
			cmd_inq_1.Parameters.Set("mat_code", cmd_inq.GetString(2));
			cmd_inq_1.ExecuteReader();
			dif_wt = 0;
			if (cmd_inq_1.Read())
			{
				dif_wt = cmd_inq.GetDecimal(3) - cmd_inq_1.GetDecimal(1);
			}
			cmd_inq_1.Close();

			//更新尾差
			if (dif_wt != 0)
			{
				sqlstr = "update tcaai06 t1 set COST_HJ = COST_HJ + @dif_wt"
					" where 1=1"
					" and data_from =@cost_center"
					" and mat_code = @mat_code"
					" and data_from=@cost_center"
					" and dept_code = @dept_code"
					" and account_period = @account_period"
					" and rownum = 1"
					" order by vfree1"
					;
				cmd_inq_1.SetCommandText(sqlstr);
				cmd_inq_1.Parameters.Set("dept_code", dept_code);
				cmd_inq_1.Parameters.Set("account_period", account_period);
				cmd_inq_1.Parameters.Set("cost_center", cmd_inq.GetString(1));
				cmd_inq_1.Parameters.Set("mat_code", cmd_inq.GetString(2));
				cmd_inq_1.Parameters.Set("dif_wt", dif_wt);
				cmd_inq_1.ExecuteNonQuery();
				cmd_inq_1.Close();
			}


			sqlstr = " update tcaai06 set cost_cj=cost_hj,cost_gj=cost_hj,cost_yj=cost_hj"
				" where 1=1"
				" and data_from =@cost_center"
				" and mat_code = @mat_code"
				" and data_from=@cost_center"
				" and dept_code = @dept_code"
				" and account_period = @account_period"
				;
			cmd_inq_1.SetCommandText(sqlstr);
			cmd_inq_1.Parameters.Set("dept_code", dept_code);
			cmd_inq_1.Parameters.Set("account_period", account_period);
			cmd_inq_1.Parameters.Set("cost_center", cmd_inq.GetString(1));
			cmd_inq_1.Parameters.Set("mat_code", cmd_inq.GetString(2));
			cmd_inq_1.ExecuteNonQuery();
			cmd_inq_1.Close();
		}
		cmd_inq.Close();

		//更新物料名称
		sqlstr = "update tcaai06 t1 set mat_name = (select mat_name from tcaac11 t2 where t1.mat_code = t2.mat_code)"
			" where 1=1"
			" and exists (select 1 from tcaac11 t2 where t1.mat_code = t2.mat_code)"			
			" AND dept_code = @dept_code"
			" AND account_period = @account_period"			
			;
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("dept_code", dept_code);
		cmd_inq.Parameters.Set("account_period", account_period);
		cmd_inq.ExecuteNonQuery();
		cmd_inq.Close();

		//将抛送的信息插入到抛帐表
		sqlstr = " delete from tcaac20"
			" where 1=1"
			" and SEND_FLAG != '1'"
			" and dept_code = @dept_code"
			" and account_period = @account_period"
			;
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("dept_code", dept_code);
		cmd_inq.Parameters.Set("account_period", account_period);
		cmd_inq.ExecuteNonQuery();
		cmd_inq.Close();

		sqlstr = " insert into tcaac20 (rec_creator, rec_create_time,account_period, dept_code, plan_no, equ_no, sub_backlog_code, mat_code, wt)"
			" select @rec_creator, @rec_create_time,@account_period, @dept_code, plan_no, equ_no, sub_backlog_code, mat_code, sum(wt)"
			" from ("
			" select vfree1 as plan_no, equ_no, sub_backlog_code, mat_code, wt"
			" from tcaai06"
			" where 1=1"
			" and mat_code in (select mat_code from tcaac11 where mat_type ='A')"
			" and dept_code = @dept_code"
			" and account_period = @account_period"
			" union all"
			" select vfree1 as plan_no, equ_no, sub_backlog_code, mat_code, 0- wt  as wt"
			" from tcaai06"
			" where 1=1"
			" and mat_code in (select mat_code from tcaac11 where mat_type ='D')"
			" and dept_code = @dept_code"
			" and account_period = @account_period"
			" union all"
			" select plan_no, equ_no, sub_backlog_code, mat_code, 0-wt as wt"
			" from tcaac20"
			" where 1=1"
			" and SEND_FLAG = '1'"			
			" and dept_code = @dept_code"
			" and account_period = @account_period"
			" )"
			" group by plan_no, equ_no, sub_backlog_code, mat_code"
			" having sum(wt)!=0"
			;
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("dept_code", dept_code);
		cmd_inq.Parameters.Set("account_period", account_period);
		cmd_inq.Parameters.Set("rec_creator", s.userid);
		cmd_inq.Parameters.Set("rec_create_time", datenow);
		cmd_inq.ExecuteNonQuery();
		cmd_inq.Close();
	}
	catch(CDbException& ex)  //捕获数据库操作异常
	{
		CString str = sqlstr + "\r\n" + ex.GetMsg();
		strncpy(s.msg, (const char*)str, sizeof(s.msg)-1);  //返回前台，与EI.EIInfo对象的sys_info.sysmsg参数对应
		s.flag = -1;
		doFlag = -1;                 //数据库异常时返回-1，事务将被回滚
	}
	catch(CApplicationException& ex)  //捕获应用错误
	{
		s.flag = ex.GetCode();
		doFlag = -1;
	}
	catch(CException& ex)
	{
		strncpy(s.msg, (const char*)ex.GetMsg(), sizeof(s.msg)-1);
		s.flag = ex.GetCode();
		doFlag = -1;
	}

	return doFlag;
}
